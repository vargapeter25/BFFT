#include "include/image.h"
#include "include/compression2d.h"
#include "include/binary_file_ops.hpp"
#include "include/argument_parser.hpp"
#include <string>
#include <unordered_map>
#include <filesystem>
#include <string>
#include <functional>

struct OptSetting{
    size_t max_iteration;
    size_t max_shrink;
};

const std::unordered_map<std::string, bfft::BlaschkeFFT::ResizeType> RESIZE_TYPES = {
    {"simple", bfft::BlaschkeFFT::ResizeType::RESIZE},
    {"linear-interpolation", bfft::BlaschkeFFT::ResizeType::LINEAR_INTERPOLATION},
};

const OptSetting opt_settings[] = {OptSetting{3, 1}, OptSetting{5, 2}, OptSetting{10, 3}, OptSetting{40, 5}};

int main(int argc, char *argv[]){
    ArgumentParser parser("compressor", "Compresses images with Blaschke Fourier-transform.");
    parser.add_argument("source").add<std::string>([](const std::string& path) { try{ return std::filesystem::exists(path); } catch(...) { ERROR(false, "Unable to reach location: " + path); return false; } }).required().help("Source file location.");
    parser.add_argument("-h").special().help("Prints command description.");
    parser.add_argument("-channels").add<int>([](int x) { return 1 <= x && x <= 4; }).help("Number of loaded channels {1=G, 2=GA, 3=RGB, 4=RGBA}, default is the picture original channels.");
    parser.add_argument("-ratio").add<double>([](double x) { return 0.0 < x && x <= 1.0; }).help("Compression ratio (double) in (0, 1], default value (0.5).");
    parser.add_argument("-resize").add<std::string>([](const std::string& s) { return RESIZE_TYPES.contains(s); }).help("Rescaling type {simple|linear-interpolation}.");
    parser.add_argument("-no-opt").help("Turns off optimization.");
    parser.add_argument("-lvl").add<int>([](int x) { return 0 <= x && x <= 3; }).help("Sets optimization level [0-3], default value 3.");
    parser.add_argument("-block").add<int>([](int x) { return 8 <= x && x <= 128; }).help("Block size in [8, 128], default value 16.");
    parser.add_argument("-name").add<std::string>().help("Save file name, location is the same as source, default name is same as source with extension `.bc`.");

    if(!parser.parse(argc - 1, argv + 1)){
        std::cerr << "Failed to parse arguments." << std::endl;
        return 1;
    }
 
    if(parser.used_argument("-h")){
        std::cout << parser.get_help();
        std::flush(std::cout);
        return 0;
    }

    int channels = 0;
    double ratio = 0.5;
    bfft::BlaschkeFFT::ResizeType resize_type = bfft::BlaschkeFFT::ResizeType::LINEAR_INTERPOLATION;
    bfft::OptimizerOpt optimizer = bfft::OptimizerOpt::NELDER_MEAD;
    size_t lvl = 3;
    size_t block_size = 16;

    std::filesystem::path file_path = parser.get_value<std::string>("source");
    std::filesystem::path save_path = std::filesystem::path(file_path).replace_extension(".bc");

    if(parser.used_argument("-channels")){
        channels = parser.get_value<int>("-channels");
    }

    if(parser.used_argument("-ratio")){
        ratio = parser.get_value<double>("-ratio");
    }

    if(parser.used_argument("-resize")){
        resize_type = RESIZE_TYPES.at(parser.get_value<std::string>("-resize"));
    }

    if(parser.used_argument("-no-opt")){
        optimizer = bfft::OptimizerOpt::NO_OPTIMIZE;
    }

    if(parser.used_argument("-lvl")){
        lvl = static_cast<size_t>(parser.get_value<int>("-lvl"));
    }

    if(parser.used_argument("-block")){
        block_size = static_cast<size_t>(parser.get_value<int>("-block"));
    }

    if(parser.used_argument("-name")){
        save_path.replace_filename(parser.get_value<std::string>("-name"));
    }

    Image image(file_path, channels);

    std::cout << "Start compressing (this may take a while)." << std::endl;
    
    auto compressed_data = image.compress(ratio, resize_type, optimizer, block_size, opt_settings[lvl].max_iteration, opt_settings[lvl].max_shrink);
    
    BinaryFileWriter fwriter(save_path);
    fwriter.write(compressed_data);
    
    std::cout << "Compressing finished." << std::endl;

    return 0;
}