#include <gtest/gtest.h>
#include <fstream>
#include <chrono>
#include "lz77.h"
#include "block.h"
#include "test-common.h"

#pragma warning(disable : 6326)

TEST(BlockTest, BasicTest) {

    auto input = readFile("tests/sample/enwik4");

    auto compressed = lpz::compress_block(input);
    if (!compressed) throw std::runtime_error("Compression failed: " + compressed.error().m);
    auto decompressed = lpz::decompress_block(*compressed);
    if (!decompressed) throw std::runtime_error("Decompression failed: " + decompressed.error().m);


    EXPECT_EQ(input, *decompressed);

}

TEST(BlockTest, EmptyTest) {

    std::vector<uint8_t> input = {};

    auto compressed = lpz::compress_block(input);

    EXPECT_EQ(compressed.error().c, lpz::ErrorCode::InputError);

    auto decompressed = lpz::decompress_block(input);
    EXPECT_EQ(decompressed.error().c, lpz::ErrorCode::InputError);

}

TEST(BlockTest, SingleChar) {

    std::vector<uint8_t> input = { 42 };

    auto compressed = lpz::compress_block(input);
    if (!compressed) throw std::runtime_error("Compression failed: " + compressed.error().m);
    auto decompressed = lpz::decompress_block(*compressed);
    if (!decompressed) throw std::runtime_error("Decompression failed: " + decompressed.error().m);

    EXPECT_EQ(input, *decompressed);

}

TEST(BlockTest, SingleCharStream) {

    std::vector<uint8_t> input = { 42,42,42,42,42,42,42,42,42 };

    auto compressed = lpz::compress_block(input);
    if (!compressed) throw std::runtime_error("Compression failed: " + compressed.error().m);
    auto decompressed = lpz::decompress_block(*compressed);
    if (!decompressed) throw std::runtime_error("Decompression failed: " + decompressed.error().m);

    EXPECT_EQ(input, *decompressed);

}


