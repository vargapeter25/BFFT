#ifndef BINARY_FILE_OPS__HPP
#define BINARY_FILE_OPS__HPP

#include "compression2d.h"
#include "mpl.hpp"
#include <string>
#include <fstream>
#include <filesystem>

// write to os

struct CompressedBlock;
struct BlockedData;

class BinaryFileWriter{
public:
    BinaryFileWriter(const std::filesystem::path &path) : m_os(path.c_str(), std::ofstream::binary) { ERROR((!m_os.fail()), "Failed to open file for writing!"); }
    ~BinaryFileWriter() { m_os.close(); }

    template<typename T>
    void write(const T& x);
    template<typename T>
    void write(const std::vector<T>& v);

private:
    std::ofstream m_os;
};

template<typename T>
void BinaryFileWriter::write(const T& x) {
    m_os.write(reinterpret_cast<const char*>(&x), sizeof(x));
    ERROR(!m_os.fail(), "Failed to write to file!");
}

template<>
void BinaryFileWriter::write<std::string>(const std::string& s);

template<>
void BinaryFileWriter::write<bfft::CompressedData2D::Coefficent>(const bfft::CompressedData2D::Coefficent& coef);

template<>
void BinaryFileWriter::write<bfft::CompressedData2D>(const bfft::CompressedData2D& data);

template<>
void BinaryFileWriter::write<CompressedBlock>(const CompressedBlock& data);

template<>
void BinaryFileWriter::write<BlockedData>(const BlockedData& data);

template<>
void BinaryFileWriter::write<Complex>(const Complex& x);

template<typename T>
void BinaryFileWriter::write(const std::vector<T>& v){
    write(v.size());
    for(const auto& x : v) write(x);
}

// read from is

class BinaryFileReader{
public:
    BinaryFileReader(const std::filesystem::path &path) : m_is(path.c_str(), std::ifstream::binary) { ERROR((!m_is.fail()), "Failed to open file for reading!"); }
    ~BinaryFileReader() { m_is.close(); }

    template<typename T>
    void read(T& x);
    template<typename T>
    void read(std::vector<T>& v);
private:
    std::ifstream m_is;
};

template<typename T>
void BinaryFileReader::read(T& x){
    m_is.read(reinterpret_cast<char*>(&x), sizeof(x));
    ERROR(!m_is.fail(), "Failed to read from file!");
}

template<>
void BinaryFileReader::read<std::string>(std::string& s);

template<>
void BinaryFileReader::read<bfft::CompressedData2D::Coefficent>(bfft::CompressedData2D::Coefficent& coef);

template<>
void BinaryFileReader::read<bfft::CompressedData2D>(bfft::CompressedData2D& data);

template<>
void BinaryFileReader::read<CompressedBlock>(CompressedBlock& data);

template<>
void BinaryFileReader::read<BlockedData>(BlockedData& data);

template<>
void BinaryFileReader::read<Complex>(Complex& x);

template<typename T>
void BinaryFileReader::read(std::vector<T>& v){
    size_t n;
    read(n);
    try{
        v.resize(n);
    } catch(...){
        ERROR(false, "Failed to read from file! (Corrupt or invalid data.)");
    }
    for(auto& x : v) read(x);
}

#endif //BINARY_WRITER__HPP