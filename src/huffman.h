#pragma once
#include <vector>
#include <span>

namespace lpz::huffman {

	std::vector<uint8_t> compress(std::span<const uint8_t> data);
	std::vector<uint8_t> decompress(std::span<const uint8_t> data);

}