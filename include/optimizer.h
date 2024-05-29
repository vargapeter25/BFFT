#ifndef OPTIMIZER__H
#define OPTIMIZER__H

#include "fft2.hpp"
#include "matrix.hpp"
#include "compression.h"
#include "compression2d.h"
#include <valarray>

namespace bfft{

enum OptimizerOpt {NO_OPTIMIZE, NELDER_MEAD};

class OptimizerFun1D{
public:
    using arg_type = double;
    using value_type = double;
    OptimizerFun1D(const std::vector<BlaschkeFFT::value_type> &data, 
                   const BlaschkeFFT& bfft,
                   size_t lvl, 
                   double ratio, 
                   BlaschkeFFT::ResizeType resize_type)
        : m_data(data), m_bfft(bfft), m_lvl(lvl), m_ratio(ratio), m_resize_type(resize_type) {}

    double operator()(const std::valarray<double> &args) const;

    size_t argc() const { return 2; }

private:
    std::vector<BlaschkeFFT::value_type> m_data;
    BlaschkeFFT m_bfft;
    size_t m_lvl;
    double m_ratio;
    BlaschkeFFT::ResizeType m_resize_type;
};

class OptimizerFun2D{
public:
    using arg_type = double;
    using value_type = double;
    using Mat = matrix::Matrix<BlaschkeFFT::value_type>;

    enum OptType {ROW, COL};
    
    OptimizerFun2D(const Mat &data, 
                   const BlaschkeFFT2 &bfft, 
                   size_t idx, 
                   size_t lvl, 
                   OptType type, 
                   double ratio, 
                   BlaschkeFFT::ResizeType resize_type)
        : m_data(data), m_bfft(bfft), m_idx(idx), m_lvl(lvl), m_type(type), m_ratio(ratio), m_resize_type(resize_type) {}

    double operator()(const std::valarray<double> &args) const;

    size_t argc() const { return 2; }

private:
    Mat m_data;
    // required for fast optimization
    mutable BlaschkeFFT2 m_bfft;
    size_t m_idx;
    size_t m_lvl;
    OptType m_type;
    double m_ratio;
    BlaschkeFFT::ResizeType m_resize_type;
};

BlaschkeFFT optimize_blaschke_fft(const std::vector<BlaschkeFFT::value_type>& data, double ratio, BlaschkeFFT::ResizeType resize_type, size_t max_iterations = 50, size_t max_shrink = 5, size_t sample_radius = 10, size_t sample_angle = 20);

BlaschkeFFT2 optimize_blaschke_fft(const matrix::Matrix<BlaschkeFFT::value_type>& data, double ratio, BlaschkeFFT::ResizeType resize_type, size_t max_iterations = 50, size_t max_shrink = 5);

}

#endif //OPTIMIZER__H