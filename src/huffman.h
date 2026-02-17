#pragma once
#include "lpz.h"
#include <vector>
#include <span>
#include <expected>

namespace lpz::huffman {

	double compute_ratio(std::span<const uint8_t> data);
	std::expected<std::vector<uint8_t>, Error> encode(std::span<const uint8_t> data);
	std::expected<std::vector<uint8_t>, Error> decode(std::span<const uint8_t> data);

}