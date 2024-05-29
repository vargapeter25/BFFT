#ifndef IMAGE_COMPRESSOR__H
#define IMAGE_COMPRESSOR__H

#include <filesystem>
#include <iostream>
#include <vector>
#include "complex.h"
#include "compression2d.h"
#include "stb_image.h"
#include "stb_image_write.h"
#include "optimizer.h"

struct CompressedBlock{
    size_t offset_row;
    size_t offset_col;
    size_t rows;
    size_t cols;
    bfft::CompressedData2D data;
};

struct BlockedData{
    std::vector<CompressedBlock> blocks;
    size_t rows;
    size_t cols;
};

class Image{
public:
    using Mat = bfft::matrix::Matrix<Complex>;
    Image(const std::filesystem::path& path, int read_channels = 0);
    Image(const std::vector<Mat>& channels);
    Image(const std::vector<BlockedData>& channels) : Image(decompress(channels)) {}
    ~Image();

    void save(const std::filesystem::path& path);
    std::vector<Mat> convert_to_mat();
    std::vector<BlockedData> compress(double ratio, 
                                      bfft::BlaschkeFFT::ResizeType resize_type, 
                                      bfft::OptimizerOpt optimizer_opt, 
                                      size_t block_size = 16, 
                                      size_t max_iteration = 40, 
                                      size_t max_shrink = 5);

    static std::vector<Mat> decompress(const std::vector<BlockedData>& channels);
private:
    unsigned char* m_image_ptr = nullptr;
    int m_width = 0;
    int m_height = 0;
    int m_channels = 0;
};

#endif //IMAGE_COMPRESSOR__H