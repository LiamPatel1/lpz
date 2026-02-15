#pragma once
#include "lpz.h"
#include <vector>
#include <span>
#include <expected>

namespace lpz::lz77 {

	std::expected<std::vector<uint8_t>, Error> compress(std::span<const uint8_t> data);
	std::expected<std::vector<uint8_t>, Error> decompress(std::span<const uint8_t> data);

}