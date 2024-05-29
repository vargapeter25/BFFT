#include "../include/binary_file_ops.hpp"
#include "../include/image.h"

template<>
void BinaryFileWriter::write<std::string>(const std::string& s){
    write(s.size());
    m_os.write(&s[0], s.size());
    ERROR(!m_os.fail(), "Failed to write to file!");
}

template<>
void BinaryFileWriter::write<bfft::CompressedData2D::Coefficent>(const bfft::CompressedData2D::Coefficent& coef){
    write(coef.id_x);
    write(coef.id_y);
    write(coef.value);
}

template<>
void BinaryFileWriter::write<bfft::CompressedData2D>(const bfft::CompressedData2D& data){
    write(data.data);
    write(data.row_params);
    write(data.col_params);
    write(data.transfomrmed_rows);
    write(data.transfomrmed_cols);
    write(data.result_rows);
    write(data.result_cols);
    write(data.resize_type);
}

template<>
void BinaryFileWriter::write<CompressedBlock>(const CompressedBlock& data){
    write(data.offset_row);
    write(data.offset_col);
    write(data.rows);
    write(data.cols);
    write(data.data);
}

template<>
void BinaryFileWriter::write<BlockedData>(const BlockedData& data){
    write(data.blocks);
    write(data.rows);
    write(data.cols);
}

template<>
void BinaryFileWriter::write<Complex>(const Complex& x){
    write(x.real);
    write(x.imag);
}

template<>
void BinaryFileReader::read<std::string>(std::string& s){
    size_t str_size;
    read(str_size);
    s.resize(str_size);
    m_is.read(&s[0], str_size);
    ERROR(!m_is.fail(), "Failed to read from file!");
}

template<>
void BinaryFileReader::read<bfft::CompressedData2D::Coefficent>(bfft::CompressedData2D::Coefficent& coef){
    read(coef.id_x);
    read(coef.id_y);
    read(coef.value);
}

template<>
void BinaryFileReader::read<bfft::CompressedData2D>(bfft::CompressedData2D& data){
    read(data.data);
    read(data.row_params);
    read(data.col_params);
    read(data.transfomrmed_rows);
    read(data.transfomrmed_cols);
    read(data.result_rows);
    read(data.result_cols);
    read(data.resize_type);
}

template<>
void BinaryFileReader::read<CompressedBlock>(CompressedBlock& data){
    read(data.offset_row);
    read(data.offset_col);
    read(data.rows);
    read(data.cols);
    read(data.data);
}

template<>
void BinaryFileReader::read<BlockedData>(BlockedData& data){
    read(data.blocks);
    read(data.rows);
    read(data.cols);
}

template<>
void BinaryFileReader::read<Complex>(Complex& x){
    read(x.real);
    read(x.imag);
}