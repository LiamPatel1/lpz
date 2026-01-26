#include <gtest/gtest.h>
#include <fstream>
#include <chrono>
#include "huffman.h"

std::vector<uint8_t> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("Failed to read file: " + filename);
    }

    return buffer;
}


TEST(HuffmanTest, BasicTest) {

    auto input = readFile("tests/sample/enwik7");

    auto compressed = lpz::huffman::compress(input);
    auto decompressed = lpz::huffman::decompress(compressed);

    EXPECT_EQ(input, decompressed);
        
}
TEST(HuffmanTest, ProfileTest) {

    using clock = std::chrono::steady_clock;

    auto input = readFile("tests/sample/enwik8");

    constexpr int runs = 5;
    std::chrono::nanoseconds comp_total{ 0 };
    std::chrono::nanoseconds decomp_total{ 0 };

    std::vector<uint8_t> compressed;
    std::vector<uint8_t> decompressed;

    for (int i = 0; i < runs; ++i) {
        auto start = clock::now();
        compressed = lpz::huffman::compress(input);
        auto mid = clock::now();
        decompressed = lpz::huffman::decompress(compressed);
        auto end = clock::now();

        comp_total += mid - start;
        decomp_total += end - mid;
    }

    EXPECT_EQ(input, decompressed);

    auto comp_avg = std::chrono::duration_cast<std::chrono::milliseconds>(comp_total / runs);
    auto decomp_avg = std::chrono::duration_cast<std::chrono::milliseconds>(decomp_total / runs);

    std::cout << "Compression ratio: "
        << (double)compressed.size() / input.size() << "\n";

    std::cout << "Avg Compression Time: " << comp_avg.count() << " ms\n";
    std::cout << "Avg Decompression Time: " << decomp_avg.count() << " ms\n";
}

