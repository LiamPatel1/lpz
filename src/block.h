#pragma once
#include <string>
#include <vector>
#include <span>
#include <expected>
#include "lpz.h"

namespace lpz {

	std::expected<std::vector<uint8_t>, Error> compress_block(std::span<const uint8_t> data);
	std::expected<std::vector<uint8_t>, Error> decompress_block(std::span<const uint8_t> data);

}