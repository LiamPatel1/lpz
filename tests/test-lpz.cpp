#include <gtest/gtest.h>
#include <fstream>
#include <chrono>
#include "lz77.h"
#include "test-common.h"

#pragma warning(disable : 6326)

TEST(LPZTest, BasicTest) {

    auto input = readFile("tests/sample/enwik7");

    auto compressed = lpz::compress(input);
    if (!compressed) throw std::runtime_error("Compression failed: " + compressed.error().m);
    auto decompressed = lpz::decompress(*compressed);
    if (!decompressed) throw std::runtime_error("Decompression failed: " + decompressed.error().m);
    EXPECT_EQ(input, *decompressed);
}

TEST(LPZTest, EmptyTest) {

    std::vector<uint8_t> input = {};

    auto compressed = lpz::compress(input);
    EXPECT_EQ(compressed.error().c, lpz::ErrorCode::InputError);
    auto decompressed = lpz::decompress(input);
    EXPECT_EQ(decompressed.error().c, lpz::ErrorCode::InputError);
}

TEST(LPZTest, SingleChar) {

    std::vector<uint8_t> input = { 42 };

    auto compressed = lpz::compress(input);
    if (!compressed) throw std::runtime_error("Compression failed: " + compressed.error().m);
    auto decompressed = lpz::decompress(*compressed);
    if (!decompressed) throw std::runtime_error("Decompression failed: " + decompressed.error().m);
    EXPECT_EQ(input, *decompressed);
}

TEST(LPZTest, SingleCharStream) {

    std::vector<uint8_t> input = { 42,42,42,42,42,42,42,42,42 };

    auto compressed = lpz::compress(input);
    if (!compressed) throw std::runtime_error("Compression failed: " + compressed.error().m);
    auto decompressed = lpz::decompress(*compressed);
    if (!decompressed) throw std::runtime_error("Decompression failed: " + decompressed.error().m);
    EXPECT_EQ(input, *decompressed);
}
