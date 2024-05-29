#include <filesystem>
#include <iostream>
#include "include/image.h"
#include "include/binary_file_ops.hpp"
#include "include/compression2d.h"
#include "include/argument_parser.hpp"

int main(int argc, char *argv[]){

    ArgumentParser parser("decompressor", "Decompresses images compressed with Blaschke Fourier-transform.");

    parser.add_argument("source").add<std::string>([](const std::string& path) { try{ return std::filesystem::exists(path); } catch(...) { ERROR(false, "Unable to reach location: " + path); return false; } }).required().help("Source file location.");
    parser.add_argument("-h").special().help("Prints command description.");
    parser.add_argument("-name").add<std::string>().help("Save file name, location is the same as source, default name is same as source with extension `.png`.");

    if(!parser.parse(argc - 1, argv + 1)){
        std::cerr << "Failed to parse arguments." << std::endl;
        return 1;
    }

    if(parser.used_argument("-h")){
        std::cout << parser.get_help();
        std::flush(std::cout);
        return 0;
    }

    std::filesystem::path file_path = parser.get_value<std::string>("source");
    std::filesystem::path save_path = std::filesystem::path(file_path).replace_extension(".png");

    if(parser.used_argument("-name")){
        save_path.replace_filename(parser.get_value<std::string>("-name"));
    }

    BinaryFileReader freader(file_path);

    std::vector<BlockedData> compressed_data;
    freader.read(compressed_data);

    std::cout << "Start decompressing." << std::endl;

    Image image(compressed_data);

    image.save(save_path);

    std::cout << "Decompressing finished." << std::endl;

    return 0;
}