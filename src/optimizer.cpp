#include "../include/optimizer.h"
#include "../include/compression.h"
#include "../include/compression2d.h"
#include "../include/nelder_mead.hpp"

#include <utility>

using namespace bfft;

double OptimizerFun1D::operator()(const std::valarray<double> &args) const {
    ASSERT((args.size() == argc()), "Argumentum counts must mach!");
    BlaschkeFFT bfft(m_bfft);

    double angle = args[0];
    double radius = std::clamp(args[1], -0.99, 0.99); // radius required to be in (-1, 1), radius close to 1 is not optimal

    bfft.function_system().set_function(m_lvl, Complex::polar(radius, angle));

    Compressor1D compressor(bfft, m_ratio, m_resize_type);
    return compressor.compression_error(m_data);
}

double OptimizerFun2D::operator()(const std::valarray<double> &args) const {
    ASSERT((args.size() == argc()), "Argumentum counts must mach!");
    // BlaschkeFFT2 bfft(m_bfft);

    //if too close to border shouldn't be optimal
    if(std::abs(args[1]) > 0.98){
        return 1e18;
    }

    double angle = args[0];
    double radius = std::clamp(args[1], -0.99, 0.99); // radius required to be in (-1, 1), radius close to 1 is not optimal

    if(m_type == OptType::ROW){
        m_bfft.get_row_fft(m_idx).function_system().set_function(m_lvl, Complex::polar(radius, angle));
    } else {
        m_bfft.get_col_fft(m_idx).function_system().set_function(m_lvl, Complex::polar(radius, angle));
    }

    // return compressor.compression_error(m_data);
    // faster with no copy
    return Compressor2D::compression_error(m_bfft, m_data, m_ratio, m_resize_type);
}

BlaschkeFFT bfft::optimize_blaschke_fft(const std::vector<BlaschkeFFT::value_type>& data, double ratio, BlaschkeFFT::ResizeType resize_type, size_t max_iterations, size_t max_shrink, size_t sample_radius, size_t sample_angle){
    std::vector<std::valarray<double>> sample_points = {
        {0, 0},
    };

    for(int i = 1; i < static_cast<int>(sample_radius); i++){
        for(int j = 1; j < static_cast<int>(sample_angle); j++){
            sample_points.push_back({std::numbers::pi / 20 * j, 0.1 * i});
        }
    }

    BlaschkeFFT bfft;
    size_t iterations = 1;
    for(size_t iter = 0; iter < iterations; iter++){
        for(size_t lvl = ceil_log2(data.size()); lvl > 0; lvl--){
            OptimizerFun1D opt_fun(data, bfft, lvl - 1, ratio, resize_type);

            std::valarray<double> origin = sample_points[0];
            double best_val = opt_fun(origin);
            for(size_t i = 1; i < sample_points.size(); i++){
                double tmp = opt_fun(sample_points[i]);
                if(tmp < best_val){
                    best_val = tmp;
                    origin = sample_points[i];
                }
            }


            std::vector<std::valarray<double>> start_points = {
                origin,
                {origin[0] + 0.1, origin[1]},
                {origin[0], origin[1] + 0.1},
            };
            
            NelderMead<OptimizerFun1D> optimizer(opt_fun, 0.001);


            auto result = optimizer.find_min(start_points, max_iterations, max_shrink);

            double angle = result[0];
            double radius = std::clamp(result[1], -0.99, 0.99);

            bfft.function_system().set_function(lvl - 1, Complex::polar(radius, angle));
        }
    }
    return bfft;
}

BlaschkeFFT2 bfft::optimize_blaschke_fft(const matrix::Matrix<BlaschkeFFT::value_type>& data, double ratio, BlaschkeFFT::ResizeType resize_type, size_t max_iterations, size_t max_shrink){
    BlaschkeFFT2 bfft2(data.rows(), data.cols());

    std::vector<std::valarray<double>> sample_points = {
        {0, 0},
    };

    static const int radius_segments = 3;
    static const int angle_segments  = 4;
    for(int i = 1; i <= angle_segments; i++){
        for(int j = 1; j <= radius_segments; j++){
            sample_points.push_back({
                std::numbers::pi / static_cast<double>(angle_segments) * static_cast<double>(i - 1),
                0.9 / static_cast<double>(radius_segments) * static_cast<double>(j),
            });
        }
    }

    for(size_t row = 0; row < data.rows(); row++){
        for(size_t lvl = 1; lvl <= ceil_log2(data.cols()); lvl++){
            OptimizerFun2D opt_fun(data, bfft2, row, lvl - 1, OptimizerFun2D::ROW, ratio, resize_type);

            std::valarray<double> origin = sample_points[0];
            double best_val = opt_fun(origin);
            for(size_t i = 1; i < sample_points.size(); i++){
                double tmp = opt_fun(sample_points[i]);
                if(tmp < best_val){
                    best_val = tmp;
                    origin = sample_points[i];
                }
            }

            std::vector<std::valarray<double>> start_points = {
                origin,
                {origin[0] + 0.1, origin[1]},
                {origin[0], origin[1] + 0.1},
            };
            
            NelderMead<OptimizerFun2D> optimizer(opt_fun, 0.001);

            auto result = optimizer.find_min(start_points, max_iterations, max_shrink);

            double angle = result[0];
            double radius = std::clamp(result[1], -0.99, 0.99);

            bfft2.get_row_fft(row).function_system().set_function(lvl - 1, Complex::polar(radius, angle));
        }
    }
    for(size_t col = 0; col < data.cols(); col++){
        for(size_t lvl = 1; lvl <= ceil_log2(data.rows()); lvl++){
            OptimizerFun2D opt_fun(data, bfft2, col, lvl - 1, OptimizerFun2D::OptType::COL, ratio, resize_type);

            std::valarray<double> origin = sample_points[0];
            double best_val = opt_fun(origin);
            for(size_t i = 1; i < sample_points.size(); i++){
                double tmp = opt_fun(sample_points[i]);
                if(tmp < best_val){
                    best_val = tmp;
                    origin = sample_points[i];
                }
            }

            std::vector<std::valarray<double>> start_points = {
                origin,
                {origin[0] + 0.1, origin[1]},
                {origin[0], origin[1] + 0.1},
            };

            NelderMead<OptimizerFun2D> optimizer(opt_fun, 0.001);

            auto result = optimizer.find_min(start_points, max_iterations, max_shrink);

            double angle = result[0];
            double radius = std::clamp(result[1], -0.99, 0.99);

            bfft2.get_col_fft(col).function_system().set_function(lvl - 1, Complex::polar(radius, angle));
        }
    }

    return bfft2;
}