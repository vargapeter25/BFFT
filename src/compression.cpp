#include "../include/compression.h"

using namespace bfft;

CompressedData1D Compressor1D::compress(const std::vector<BlaschkeFunction::value_type> &source) const {
    auto transformed_data = m_bfft.fft(source, m_resize_type);
    std::vector<CompressedData1D::Coefficent> coefs(transformed_data.size());
    for(size_t i = 0; i < transformed_data.size(); i++) {
        coefs[i] = CompressedData1D::Coefficent{i, transformed_data[i]};
    }
    std::sort(coefs.rbegin(), coefs.rend(), [](const CompressedData1D::Coefficent &coef1, const CompressedData1D::Coefficent &coef2) {
        double value_abs1 = Complex::abs(coef1.value);
        double value_abs2 = Complex::abs(coef2.value);
        return value_abs1 != value_abs2 ? value_abs1 < value_abs2 : coef1.id < coef2.id;
    });
    size_t split = std::min(static_cast<size_t>(static_cast<double>(transformed_data.size()) * m_ratio), coefs.size());
    coefs.resize(split);
    return CompressedData1D{coefs, m_bfft.function_system().get_function_params(), transformed_data.size(), source.size(), m_resize_type};
}

std::vector<BlaschkeFunction::value_type> Compressor1D::decompress(const CompressedData1D& data) const {
    FunctionSystem func_sys(data.args);
    BlaschkeFFT bfft(func_sys);
    
    std::vector<BlaschkeFunction::value_type> transfomred_data(data.transformed_size, static_cast<BlaschkeFunction::value_type>(0));
    
    for(auto [id, value] : data.compr_data){
        transfomred_data[id] = value;
    }

    auto result = bfft.ifft(transfomred_data, data.original_size, data.resize_type);
    return result;
}

std::vector<BlaschkeFunction::value_type> Compressor1D::this_decompress(const CompressedData1D& data) const {
    std::vector<BlaschkeFunction::value_type> transfomred_data(data.transformed_size, static_cast<BlaschkeFunction::value_type>(0));
    
    for(auto [id, value] : data.compr_data){
        transfomred_data[id] = value;
    }

    auto result = m_bfft.ifft(transfomred_data, data.original_size, m_resize_type);
    return result;
}

double Compressor1D::compression_error(const std::vector<BlaschkeFunction::value_type>& data) const {
    auto compression_result = compress(data);
    auto decompression_result = this_decompress(compression_result);
    return mean_squared_error(data, decompression_result);
}