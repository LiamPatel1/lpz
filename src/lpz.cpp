#include "lpz.h"
#include "block.h"
#include <format>


std::expected<std::vector<uint8_t>, lpz::Error> lpz::compress(std::span<const uint8_t> data) {

	if (data.size() == 0) {
		return std::unexpected(Error{ ErrorCode::InputError, "Input block empty" });
	}

	std::vector<uint8_t> out;


	std::vector < std::span<const uint8_t> > in_blocks;

	const uint8_t const* in_end = data.data() + data.size();
	const uint8_t* in_pos = data.data();

	while (in_pos < in_end) {

		size_t block_size = std::min(MAX_BLOCK, static_cast<size_t>(in_end - in_pos));

		in_blocks.push_back( { in_pos , block_size } );

		in_pos += block_size;

	}

	for (auto& in_block : in_blocks) {

		auto comp_res = lpz::compress_block(in_block);
		if (!comp_res) return std::unexpected(Error{ ErrorCode::SystemError, "Block compression failed: " + comp_res.error().m });
		auto& comp = *comp_res;

		uint32_t size = comp.size();
		out.insert(out.end(), reinterpret_cast<uint8_t*>(&size), reinterpret_cast<uint8_t*>(&size) + sizeof(size));

		out.insert(out.end(), comp.begin(), comp.end());
	}

	return out;

}

std::expected<std::vector<uint8_t>, lpz::Error> lpz::decompress(std::span<const uint8_t> data) {

	if (data.size() == 0) {
		return std::unexpected(Error{ ErrorCode::InputError, "Input block empty" });
	}

	std::vector<uint8_t> out;

	std::vector < std::span<const uint8_t> > in_blocks;


	const uint8_t const* in_begin = data.data();
	const uint8_t const* in_end = in_begin + data.size();
	const uint8_t* in_pos = in_begin;

	while (in_pos < in_end) {

		uint32_t block_size;
		memcpy(&block_size, in_pos, sizeof(block_size));

		in_pos += sizeof(block_size);

		in_blocks.push_back({ in_pos , block_size });

		in_pos += block_size;

	}

	for (auto& in_block : in_blocks) {

		auto comp_res = lpz::decompress_block(in_block);
		if (!comp_res) return std::unexpected(Error{ ErrorCode::SystemError, "Block decompression failed: " + comp_res.error().m });
		auto& comp = *comp_res;

		out.insert(out.end(), comp.begin(), comp.end());
	}

	return out;

}