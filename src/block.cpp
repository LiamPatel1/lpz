#pragma once
#include <string>
#include <vector>
#include <span>
#include <expected>
#include "block.h"
#include "lz77.h"
#include "huffman.h"
#include <stdexcept>



std::expected<std::vector<uint8_t>, lpz::Error> lpz::compress_block(std::span<const uint8_t> data) {

	if (data.size() > MAX_BLOCK) {
		return std::unexpected(Error{ ErrorCode::InputError, "Input block too large" });
	}
	if (data.size() == 0) {
		return std::unexpected(Error{ ErrorCode::InputError, "Input block empty" });
	}

	auto lz77_comp = lpz::lz77::encode(data);
	if (!lz77_comp) throw std::runtime_error("Compression failed: " + lz77_comp.error().m);
	auto huffman_comp = lpz::huffman::encode(*lz77_comp);
	if (!huffman_comp) throw std::runtime_error("Compression failed: " + huffman_comp.error().m);
	return *huffman_comp;
	
}

std::expected<std::vector<uint8_t>, lpz::Error> lpz::decompress_block(std::span<const uint8_t> data) {

	if (data.size() == 0) {
		return std::unexpected(Error{ ErrorCode::InputError, "Input block empty" });
	}

	auto lz77_comp = lpz::huffman::decode(data);
	if (!lz77_comp) return std::unexpected(Error{ ErrorCode::InputError,"Decompression failed: " + lz77_comp.error().m });
	auto decomp = lpz::lz77::decode(*lz77_comp);
	if (!decomp) return std::unexpected(Error{ ErrorCode::InputError,"Decompression failed: " + decomp.error().m });
	return *decomp;
}