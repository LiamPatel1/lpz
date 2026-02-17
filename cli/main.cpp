#include <string>
#include <vector>
#include <ranges>
#include <iostream>
#include <filesystem>
#include "io.h"
#include <chrono>
#include "lpz.h"

void print_usage() {

    std::cout <<
R"(
Commands:
    compress [input file] [output file (optional)] 
    decompress [input file] [output file (optional)] 

)";


}

int compress(std::filesystem::path input_file, std::optional<std::filesystem::path> output_file) {

    auto in_res = read_file(input_file);
    if (!in_res) {
        std::cout << "Error reading file: " << in_res.error() << "\n";
        return 1;
    }

    auto& in = *in_res;

    auto comp_res = lpz::compress(in);
    if (!comp_res) {
        std::cout << "Error compressing: " << comp_res.error().m << "\n";
        return 1;
    }

    auto output_file_ = output_file.value_or(
        std::filesystem::path(input_file).replace_extension(".lpz")
    );

    auto res = write_file(output_file_, *comp_res, false);
    if (!res) {
        std::cout << "Error writing file: " << res.error() << "\n";
        return 1;
    }

    return 0;
}

int decompress(std::filesystem::path input_file, std::optional<std::filesystem::path> output_file) {


    auto in_res = read_file(input_file);
    if (!in_res) {
        std::cout << "Error reading file: " << in_res.error() << "\n";
        return 1;
    }

    auto& in = *in_res;

    auto comp_res = lpz::decompress(in);
    if (!comp_res) {
        std::cout << "Error decompressing: " << comp_res.error().m << "\n";
        return 1;
    }

    std::filesystem::path output_file_;
    if (!output_file) {
        if (input_file.extension() == ".lpz") {
            output_file_ = input_file.replace_extension();
        }
        else {
            std::cout << "Cannot infer output path: " << input_file.string() << "\n";
            return 1;
        }
    }
    else {
        output_file_ = *output_file;
    }


    auto res = write_file(output_file_, *comp_res, false);
    if (!res) {
        std::cout << "Error writing file: " << res.error() << "\n";
        return 1;
    }

    return 0;
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

   
    if (argv[1] == std::string("compress")) {

        if (argc == 3) {
            return compress(argv[2], std::nullopt);
        }
        else if (argc == 4) {
            return compress(argv[2], argv[3]);
        }
        else {
            std::cout << "Error: Invalid argument count\n";
            print_usage();
            return 1;
        }
    }
    else if (argv[1] == std::string("decompress")) {

        if (argc == 3) {
            return decompress(argv[2], std::nullopt);
        }
        else if (argc == 4) {
            return decompress(argv[2], argv[3]);
        }
        else {
            std::cout << "Error: Invalid argument count\n";
            print_usage();
            return 1;
        }
    }


    else {
        std::cout << "Error: Invalid command\n";
        print_usage();
        return 1;
    }

    return 0;


 }



