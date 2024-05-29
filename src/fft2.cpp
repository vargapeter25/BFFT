#include "../include/fft2.hpp"

using namespace bfft;

void BlaschkeFFT2::set_fft_rows(const std::vector<BlaschkeFFT>& ffts) {
    m_fft_rows = ffts;
}

void BlaschkeFFT2::set_fft_cols(const std::vector<BlaschkeFFT>& ffts) {
    m_fft_cols = ffts;
}

BlaschkeFFT2::value_type BlaschkeFFT2::fft(const value_type& mat, ResizeType resize_type) const {
    size_t rows = ceil_pow2(mat.rows());
    size_t cols = ceil_pow2(mat.cols());

    value_type result(rows, cols);
    value_type::copy_to(result, mat);

    fft_rows(result, mat.cols(), resize_type);
    fft_cols(result, mat.rows(), resize_type);
    return result;
}

BlaschkeFFT2::value_type BlaschkeFFT2::ifft(const value_type& mat, size_t out_rows, size_t out_cols, ResizeType resize_type) const {
    size_t rows = ceil_pow2(mat.rows());
    size_t cols = ceil_pow2(mat.cols());

    if(out_rows == 0) out_rows = rows;
    if(out_cols == 0) out_cols = cols;
    
    value_type result(rows, cols);
    value_type::copy_to(result, mat);

    ifft_cols(result, mat.rows(), out_rows, resize_type);
    ifft_rows(result, mat.cols(), out_cols, resize_type);

    if(out_rows != rows || out_cols != cols){
        value_type resized_result(out_rows, out_cols);
        value_type::copy_to(resized_result, result);
        return resized_result;
    }

    return result;
}

void BlaschkeFFT2::fft_linear_sub_matrix(const BlaschkeFFT& bfft, BlaschkeFFT2::value_type::LinearSubMatrixWrapper sub_matrix, size_t in_size, ResizeType resize_type) const {
    auto fft_result = bfft.fft(sub_matrix.begin(), sub_matrix.begin() + in_size, resize_type);
    value_type::copy_to(sub_matrix, fft_result.begin(), fft_result.end());
}

void BlaschkeFFT2::ifft_linear_sub_matrix(const BlaschkeFFT& bfft, value_type::LinearSubMatrixWrapper sub_matrix, size_t in_size, size_t out_size, ResizeType resize_type) const {
    auto ifft_result = bfft.ifft(sub_matrix.begin(), sub_matrix.begin() + in_size, out_size, resize_type);
    value_type::copy_to(sub_matrix, ifft_result.begin(), ifft_result.end());
}

void BlaschkeFFT2::fft_rows(value_type& mat, size_t in_size, ResizeType resize_type) const {
    for(size_t i = 0; i < mat.rows(); i++){
        fft_linear_sub_matrix(get_row_fft(i), mat.get_row(i), in_size, resize_type);
    }
}

void BlaschkeFFT2::ifft_rows(value_type& mat, size_t in_size, size_t out_size, ResizeType resize_type) const {
    for(size_t i = 0; i < mat.rows(); i++){
        ifft_linear_sub_matrix(get_row_fft(i), mat.get_row(i), in_size, out_size, resize_type);
    }
}

void BlaschkeFFT2::fft_cols(value_type& mat, size_t in_size, ResizeType resize_type) const {
    for(size_t i = 0; i < mat.cols(); i++){
        fft_linear_sub_matrix(get_col_fft(i), mat.get_col(i), in_size, resize_type);
    }
}

void BlaschkeFFT2::ifft_cols(value_type& mat, size_t in_size, size_t out_size, ResizeType resize_type) const {
    for(size_t i = 0; i < mat.cols(); i++){
        ifft_linear_sub_matrix(get_col_fft(i), mat.get_col(i), in_size, out_size, resize_type);
    }
}