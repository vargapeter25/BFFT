#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../include/image.h"
#include "../include/mpl.hpp"
#include "../include/compression2d.h"
#include "../include/binary_file_ops.hpp"
#include "../include/optimizer.h"
#include <thread>
#include <future>
#include <utility>

Image::Image(const std::filesystem::path& path, int read_channels){
    ASSERT((0 <= read_channels && read_channels <= 4), "Channels count must be in range [1, 4]!");
    int original_channels = 1;
    std::string path_str = path.string();
    m_image_ptr = stbi_load(path_str.c_str(), &m_width, &m_height, &original_channels, read_channels);
    
    ERROR(m_image_ptr != nullptr, "Error while loading image!");
    
    if(read_channels == 0){ //default channel count
        read_channels = original_channels;
    }
    m_channels = read_channels;
}

Image::Image(const std::vector<Mat>& channels) {
    m_height = channels[0].rows();
    m_width = channels[0].cols();
    m_channels = channels.size();
    m_image_ptr = static_cast<unsigned char*>(malloc(sizeof(unsigned char) * m_height * m_width * m_channels));
    for(int i = 0; i < m_height; i++){
        for(int j = 0; j < m_width; j++){
            for(int channel = 0; channel < m_channels; channel++){
                int idx = (i * m_width + j) * m_channels + channel;
                m_image_ptr[idx] = static_cast<unsigned char>(std::clamp(channels[channel][i][j].real + 128.0, 0.0, 255.0));
            }
        }
    }
}

std::vector<Image::Mat> Image::decompress(const std::vector<BlockedData>& channels){
    std::vector<Mat> data(channels.size(), Mat(channels[0].rows, channels[0].cols));
    bfft::Compressor2D compressor(1, 1, 1.0, bfft::BlaschkeFFT::ResizeType::RESIZE);
    for(size_t channel = 0; channel < channels.size(); channel++){
        for(const CompressedBlock& block : channels[channel].blocks){
            Mat block_mat = compressor.decompress(block.data);
            //No need to resize block, because only edges can be too big
            Mat::copy_to_pos(data[channel], block_mat, block.offset_row, block.offset_col);
        }
    }
    return data;
}

Image::~Image() { 
    free(m_image_ptr);
}

std::vector<Image::Mat> Image::convert_to_mat() {
    std::vector<Mat> result(m_channels, Mat(m_height, m_width));
    for(int i = 0; i < m_height; i++){
        for(int j = 0; j < m_width; j++){
            for(int channel = 0; channel < m_channels; channel++){
                int idx = (i * m_width + j) * m_channels + channel;
                result[channel][i][j] = Complex(m_image_ptr[idx] - 128);
            }
        }
    }
    return result;
}

std::vector<BlockedData> Image::compress(double ratio, bfft::BlaschkeFFT::ResizeType resize_type, bfft::OptimizerOpt optimizer_opt, size_t block_size, size_t max_iteration, size_t max_shrink) {
    std::vector<Mat> channels = convert_to_mat();
    std::vector<BlockedData> compressed_channels(channels.size());
    
    std::vector<std::pair<bfft::CompressedData2D&, Mat>> blocks;
    // std::vector<Mat> blocks;
    for(size_t channel = 0; channel < channels.size(); channel++){
        compressed_channels[channel].rows = channels[channel].rows();
        compressed_channels[channel].cols = channels[channel].cols();
        
        compressed_channels[channel].blocks.reserve((channels[channel].rows() / block_size + 1) * (channels[channel].cols() / block_size + 1));
        
        for(size_t i = 0; i < channels[channel].rows(); i += block_size){
            for(size_t j = 0; j < channels[channel].cols(); j += block_size){
                Mat block_mat = Mat::submatrix_from_pos(channels[channel], i, j, block_size, block_size);
                size_t rows = std::min(block_size, channels[channel].rows() - i);
                size_t cols = std::min(block_size, channels[channel].cols() - j);
                
                compressed_channels[channel].blocks.push_back(CompressedBlock{i, j, rows, cols, bfft::CompressedData2D{}});
                blocks.emplace_back(compressed_channels[channel].blocks.back().data, block_mat);
            }
        }
    }

    auto compress_block = [optimizer_opt, ratio, resize_type, max_iteration, max_shrink](Mat block) -> bfft::CompressedData2D {
        bfft::BlaschkeFFT2 bfft(block.rows(), block.cols());
        if(optimizer_opt == bfft::OptimizerOpt::NELDER_MEAD) bfft = bfft::optimize_blaschke_fft(block, ratio, resize_type, max_iteration, max_shrink);
        bfft::CompressedData2D result = bfft::Compressor2D(bfft, ratio, resize_type).compress(block);
        return result;
    };

    std::vector<std::future<bfft::CompressedData2D>> task_list;

    for(size_t i = 0; i < blocks.size(); i++){
        task_list.push_back(std::async(std::launch::async, compress_block, blocks[i].second));
    }

    try{
        for(size_t i = 0; i < blocks.size(); i++){
            blocks[i].first = task_list[i].get();
        }
    } catch(const std::exception& e){
        std::cout << "Error while waiting for blocks to be compressed: " << e.what() << std::endl;
        std::abort();
    }

    return compressed_channels;
}

void Image::save(const std::filesystem::path& path) {
    std::string path_str = path.string();
    stbi_write_png(path_str.c_str(), m_width, m_height, m_channels, m_image_ptr, m_width * m_channels * sizeof(unsigned char));
}