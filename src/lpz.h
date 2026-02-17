#pragma once
#include <string>
#include <vector>
#include <span>
#include <expected>

namespace lpz {

	constexpr size_t MAX_BLOCK = 128 * 1024;

	enum class ErrorCode {
		SystemError,
		InputError,
	};

	struct Error {
		ErrorCode c;
		std::string m;
	};


	std::expected<std::vector<uint8_t>, Error> compress(std::span<const uint8_t> data);
	std::expected<std::vector<uint8_t>, Error> decompress(std::span<const uint8_t> data);
}