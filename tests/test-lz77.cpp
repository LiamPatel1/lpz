#include <gtest/gtest.h>
#include <fstream>
#include <chrono>
#include "lz77.h"
#include "test-common.h"

#pragma warning(disable : 6326)


TEST(LZ77Test, BasicTest) {

    auto input = readFile("tests/sample/enwik5");

    auto compressed = lpz::lz77::compress(input);
    if (!compressed) throw std::runtime_error("Compression failed: " + compressed.error().m);

    auto decompressed = lpz::lz77::decompress(*compressed);
    if (!decompressed) throw std::runtime_error("Decompression failed: " + decompressed.error().m);

    EXPECT_EQ(input, *decompressed);

}


TEST(LZ77Test, ProfileTest) {

    using clock = std::chrono::steady_clock;

    auto input = readFile("tests/sample/enwik6");

    constexpr int runs = 5;
    std::chrono::nanoseconds comp_total{ 0 };
    std::chrono::nanoseconds decomp_total{ 0 };

    std::vector<uint8_t> compressed;
    std::vector<uint8_t> decompressed;

    for (int i = 0; i < runs; ++i) {
        auto start = clock::now();
        auto compressed_res = lpz::lz77::compress(input);
        if (!compressed_res) throw std::runtime_error("Compression failed: " + compressed_res.error().m);
        compressed = *compressed_res;

        auto mid = clock::now();
        auto decompressed_res = lpz::lz77::decompress(compressed);
        if (!decompressed_res) throw std::runtime_error("Decompression failed: " + decompressed_res.error().m);
        decompressed = *decompressed_res;
        auto end = clock::now();

        comp_total += mid - start;
        decomp_total += end - mid;

        EXPECT_EQ(input, decompressed);

    }

    auto comp_avg = std::chrono::duration_cast<std::chrono::milliseconds>(comp_total / runs);
    auto decomp_avg = std::chrono::duration_cast<std::chrono::microseconds>(decomp_total / runs);

    std::cout << "Compression ratio: "
        << (double)compressed.size() / input.size() << "\n";

    std::cout << "Avg Compression Time: " << comp_avg.count() << " ms\n";
    std::cout << "Avg Decompression Time: " << decomp_avg.count() << " µs\n";
}
