#include <gtest/gtest.h>
#include <fstream>
#include <chrono>
#include "lz77.h"
#include "test-common.h"

#pragma warning(disable : 6326)

TEST(LZ77Test, BasicTest) {

    auto input = readFile("tests/sample/enwik7");

    auto compressed = lpz::lz77::encode(input);
    if (!compressed) throw std::runtime_error("Compression failed: " + compressed.error().m);
    auto decompressed = lpz::lz77::decode(*compressed);
    if (!decompressed) throw std::runtime_error("Decompression failed: " + decompressed.error().m);
    EXPECT_EQ(input, *decompressed);

}

TEST(LZ77Test, EmptyTest) {

    std::vector<uint8_t> input = {};

    auto compressed = lpz::lz77::encode(input);
    EXPECT_EQ(compressed.error().c, lpz::ErrorCode::InputError);
    auto decompressed = lpz::lz77::decode(input);
    EXPECT_EQ(decompressed.error().c, lpz::ErrorCode::InputError);
}

TEST(LZ77Test, SingleChar) {

    std::vector<uint8_t> input = { 42 };

    auto compressed = lpz::lz77::encode(input);
    if (!compressed) throw std::runtime_error("Compression failed: " + compressed.error().m);
    auto decompressed = lpz::lz77::decode(*compressed);
    if (!decompressed) throw std::runtime_error("Decompression failed: " + decompressed.error().m);
    EXPECT_EQ(input, *decompressed);
}

TEST(LZ77Test, SingleCharStream) {

    std::vector<uint8_t> input = { 42,42,42,42,42,42,42,42,42 };

    auto compressed = lpz::lz77::encode(input);
    if (!compressed) throw std::runtime_error("Compression failed: " + compressed.error().m);
    auto decompressed = lpz::lz77::decode(*compressed);
    if (!decompressed) throw std::runtime_error("Decompression failed: " + decompressed.error().m);
    EXPECT_EQ(input, *decompressed);
}
