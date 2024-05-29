#ifndef COMPRESSION2D__H
#define COMPRESSION2D__H

#include "matrix.hpp"
#include "fft.hpp"
#include "fft2.hpp"
#include <fstream>

namespace bfft{

struct CompressedData2D{
    struct Coefficent{
        size_t id_x;
        size_t id_y;
        BlaschkeFunction::value_type value;
        bool operator<(const Coefficent& coef) const;
    };
    std::vector<Coefficent> data;
    std::vector<std::vector<BlaschkeFunction::value_type>> row_params;
    std::vector<std::vector<BlaschkeFunction::value_type>> col_params;
    size_t transfomrmed_rows;
    size_t transfomrmed_cols;
    size_t result_rows;
    size_t result_cols;
    bfft::BlaschkeFFT::ResizeType resize_type;
};

class Compressor2D{
public:
    Compressor2D(const std::vector<std::vector<BlaschkeFunction::value_type>> &row_params, 
                 const std::vector<std::vector<BlaschkeFunction::value_type>> &col_params, 
                 double ratio,
                 BlaschkeFFT::ResizeType resize_type = BlaschkeFFT::ResizeType::RESIZE);
    Compressor2D(const BlaschkeFFT2 &bfft, double ratio, BlaschkeFFT::ResizeType resize_type = BlaschkeFFT::ResizeType::RESIZE)
        : m_bfft(bfft), m_ratio(ratio), m_resize_type(resize_type) { ASSERT((0 < ratio && ratio <= 1.0), "Ratio is not between boundaries (0, 1]!"); }
    Compressor2D(size_t rows, size_t cols, double ratio, 
                 BlaschkeFFT::ResizeType resize_type = BlaschkeFFT::ResizeType::RESIZE) 
        : m_bfft(rows, cols), m_ratio(ratio), m_resize_type(resize_type) { ASSERT((0 < ratio && ratio <= 1.0), "Ratio is not between boundaries (0, 1]!"); }

    CompressedData2D compress(const matrix::Matrix<BlaschkeFunction::value_type>& source) const;
    matrix::Matrix<BlaschkeFunction::value_type> decompress(const CompressedData2D& data) const;
    matrix::Matrix<BlaschkeFunction::value_type> this_decompress(const CompressedData2D& data) const;

    double compression_error(const matrix::Matrix<BlaschkeFunction::value_type>& data) const;

    BlaschkeFFT2 get_bfft() const { return m_bfft; }

    static double compression_error(const BlaschkeFFT2& bfft, const matrix::Matrix<BlaschkeFunction::value_type>& data, double ratio, BlaschkeFFT::ResizeType resize_type);
private:
    bfft::BlaschkeFFT2 m_bfft;
    double m_ratio;
    BlaschkeFFT::ResizeType m_resize_type;
};

}

#endif //COMPRESSION2D__H