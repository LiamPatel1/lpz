#include <gtest/gtest.h>
#include <fstream>
#include <chrono>
#include "huffman.h"
#include "test-common.h"

#pragma warning(disable : 6326)

TEST(HuffmanTest, BasicTest) {

    auto input = readFile("tests/sample/enwik6");

    auto compressed = lpz::huffman::encode(input);
    if (!compressed) throw std::runtime_error("Compression failed: " + compressed.error().m);
    auto decompressed = lpz::huffman::decode(*compressed);
    if (!decompressed) throw std::runtime_error("Decompression failed: " + decompressed.error().m);

    EXPECT_EQ(input, *decompressed);
}

TEST(HuffmanTest, EmptyTest) {

    std::vector<uint8_t> input = {};

    auto compressed = lpz::huffman::encode(input);

    EXPECT_EQ(compressed.error().c, lpz::ErrorCode::InputError);

    auto decompressed = lpz::huffman::decode(input);
    EXPECT_EQ(decompressed.error().c, lpz::ErrorCode::InputError);

}

TEST(HuffmanTest, SingleChar) {

    std::vector<uint8_t> input = {42};

    auto compressed = lpz::huffman::encode(input);
    if (!compressed) throw std::runtime_error("Compression failed: " + compressed.error().m);
    auto decompressed = lpz::huffman::decode(*compressed);
    if (!decompressed) throw std::runtime_error("Decompression failed: " + decompressed.error().m);

    EXPECT_EQ(input, *decompressed);

}

TEST(HuffmanTest, SingleCharStream) {

    std::vector<uint8_t> input = { 42,42,42,42,42,42,42,42,42 };

    auto compressed = lpz::huffman::encode(input);
    if (!compressed) throw std::runtime_error("Compression failed: " + compressed.error().m);
    auto decompressed = lpz::huffman::decode(*compressed);
    if (!decompressed) throw std::runtime_error("Decompression failed: " + decompressed.error().m);

    EXPECT_EQ(input, *decompressed);

}

