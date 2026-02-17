#include <iostream>
#include <fstream>
#include <benchmark/benchmark.h>

std::vector<uint8_t> g_input;


int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: lpz-benchmark.exe <input_file>\n";
        return 1;
    }

    std::ifstream file(argv[1], std::ios::binary);
    if (!file) {
        std::cout << "Failed to open file\n";
        return 1;
    }

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0);

    g_input.resize(size);
    file.read(reinterpret_cast<char*>(g_input.data()), size);

    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
}