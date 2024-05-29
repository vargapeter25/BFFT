#include "../include/compression2d.h"

using namespace bfft;

bool CompressedData2D::Coefficent::operator<(const Coefficent& coef) const {
    double abs1 = Complex::abs(value);
    double abs2 = Complex::abs(coef.value);
    return abs1 != abs2 ? abs1 < abs2 : (id_x != coef.id_x ? id_x < coef.id_x : id_y < coef.id_y);
}

Compressor2D::Compressor2D(const std::vector<std::vector<BlaschkeFunction::value_type>> &row_params,
                           const std::vector<std::vector<BlaschkeFunction::value_type>> &col_params,
                           double ratio, BlaschkeFFT::ResizeType resize_type) 
    : m_bfft(row_params.size(), col_params.size()), m_ratio(ratio), m_resize_type(resize_type)
{
    std::vector<BlaschkeFFT> fft_rows(row_params.size());
    std::vector<BlaschkeFFT> fft_cols(col_params.size());
    for(size_t i = 0; i < row_params.size(); i++) fft_rows[i] = BlaschkeFFT(FunctionSystem(row_params[i]));
    for(size_t i = 0; i < col_params.size(); i++) fft_cols[i] = BlaschkeFFT(FunctionSystem(col_params[i]));
    m_bfft.set_fft_rows(fft_rows);
    m_bfft.set_fft_cols(fft_cols);
}

CompressedData2D Compressor2D::compress(const matrix::Matrix<BlaschkeFunction::value_type>& source) const {
    auto transformed_data = m_bfft.fft(source, m_resize_type);
    std::vector<CompressedData2D::Coefficent> coefs(transformed_data.rows() * transformed_data.cols());
    for(size_t i = 0; i < transformed_data.rows(); i++){
        for(size_t j = 0; j < transformed_data.cols(); j++){
            coefs[i * transformed_data.cols() + j] = {i, j, transformed_data[i][j]};
        }
    }
    std::sort(coefs.rbegin(), coefs.rend());
    size_t split = std::min(static_cast<size_t>(coefs.size() * m_ratio), coefs.size());
    coefs.resize(split);
    std::vector<std::vector<BlaschkeFunction::value_type>> row_params(m_bfft.rows());
    std::vector<std::vector<BlaschkeFunction::value_type>> col_params(m_bfft.cols());
    for(size_t i = 0; i < m_bfft.rows(); i++){
        row_params[i] = m_bfft.get_row_fft(i).function_system().get_function_params();
    }
    for(size_t i = 0; i < m_bfft.cols(); i++){
        col_params[i] = m_bfft.get_col_fft(i).function_system().get_function_params();
    }
    return CompressedData2D{coefs, row_params, col_params, transformed_data.rows(), transformed_data.cols(), source.rows(), source.cols(), m_resize_type};  
}

matrix::Matrix<BlaschkeFunction::value_type> Compressor2D::decompress(const CompressedData2D& data) const {
    matrix::Matrix<BlaschkeFunction::value_type> transformed_data(data.transfomrmed_rows, data.transfomrmed_cols);
    for(auto [id_x, id_y, value] : data.data){
        transformed_data[id_x][id_y] = value;
    }
    std::vector<BlaschkeFFT> row_ffts(data.row_params.size());
    std::vector<BlaschkeFFT> col_ffts(data.col_params.size());
    for(size_t i = 0; i < data.row_params.size(); i++) row_ffts[i] = BlaschkeFFT(data.row_params[i]);
    for(size_t i = 0; i < data.col_params.size(); i++) col_ffts[i] = BlaschkeFFT(data.col_params[i]);
    BlaschkeFFT2 bfft(row_ffts, col_ffts);
    auto result = bfft.ifft(transformed_data, data.result_rows, data.result_cols, data.resize_type);
    return result;
}

matrix::Matrix<BlaschkeFunction::value_type> Compressor2D::this_decompress(const CompressedData2D& data) const {
    matrix::Matrix<BlaschkeFunction::value_type> transformed_data(data.transfomrmed_rows, data.transfomrmed_cols);
    for(auto [id_x, id_y, value] : data.data){
        transformed_data[id_x][id_y] = value;
    }
    auto result = m_bfft.ifft(transformed_data, data.result_rows, data.result_cols, data.resize_type);
    return result;
}

double Compressor2D::compression_error(const matrix::Matrix<BlaschkeFunction::value_type>& data) const {
    auto compressed_data = compress(data);
    auto result = this_decompress(compressed_data);
    return mean_squared_error(data.data(), result.data());
}

 double Compressor2D::compression_error(const BlaschkeFFT2& bfft, const matrix::Matrix<BlaschkeFunction::value_type>& data, double ratio, BlaschkeFFT::ResizeType resize_type) {
    auto transformed_data = bfft.fft(data, resize_type);
    std::vector<CompressedData2D::Coefficent> coefs(transformed_data.rows() * transformed_data.cols());
    for(size_t i = 0; i < transformed_data.rows(); i++){
        for(size_t j = 0; j < transformed_data.cols(); j++){
            coefs[i * transformed_data.cols() + j] = {i, j, transformed_data[i][j]};
        }
    }
    std::sort(coefs.rbegin(), coefs.rend());
    size_t split = std::min(static_cast<size_t>(coefs.size() * ratio), coefs.size());
    for(size_t i = std::max(split, static_cast<size_t>(0)); i < coefs.size(); i++) transformed_data[coefs[i].id_x][coefs[i].id_y] = BlaschkeFunction::value_type(0);
    auto result = bfft.ifft(transformed_data, data.rows(), data.cols(), resize_type);
    return mean_squared_error(data.data(), result.data());
 }