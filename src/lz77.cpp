#include "lz77.h"
#include <cassert>
#include <iostream>

namespace {

	constexpr uint16_t MAX_DISTANCE = 65535;
	constexpr uint32_t MAX_LENGTH = 2 * 1024;

	constexpr int MAX_CHAIN = 64;

	constexpr int MIN_MATCH = 4;
	constexpr int MATCH_LENGTH_BIAS = MIN_MATCH;

	constexpr uint32_t HASH_BITS = 15;
	constexpr uint32_t HASH_SIZE = 1 << HASH_BITS;

	static_assert(MIN_MATCH >= MATCH_LENGTH_BIAS);

	inline uint32_t hash(const uint8_t* p) {
		uint32_t val = *(const uint32_t*)p; 
		val *= 0x1e35a7bd;                 
		return (val >> (32 - HASH_BITS)) & ((1u << HASH_BITS) - 1);
	}
}

std::expected<std::vector<uint8_t>, lpz::Error> 
lpz::lz77::compress(std::span<const uint8_t> input) {

	if (input.size() >= std::numeric_limits<uint32_t>::max())
		return std::unexpected(Error{ ErrorCode::InputError, "LZ77 compress: Input too large" });
	if (input.empty())
		return std::unexpected(Error{ ErrorCode::InputError, "LZ77 compress: Empty Input" });
	
	std::vector<int32_t> head(HASH_SIZE, -1);
	std::vector<int32_t> chain(input.size(), -1);

	std::vector<uint8_t> output;
	output.reserve(input.size());

	const uint8_t* const in_base = input.data();
	const uint8_t* ip = in_base;
	const uint8_t* const in_end = in_base + input.size();
	const uint8_t* anchor = ip;

	while (ip < in_end) {

		if (ip + std::max(3, MIN_MATCH) >= in_end) {
			ip++;
			continue;
		}

		uint32_t best_length = 0;
		uint16_t best_distance = 0;

		uint32_t next_hash = hash(ip);

		uint32_t prev = head[next_hash];
		head[next_hash] = static_cast<uint32_t>(ip - in_base);

		chain[ip - in_base] = prev;

		int chain_depth = 0;

		while (prev != -1 && chain_depth < MAX_CHAIN) {
	
			uint32_t length = 0;

			if (ip - in_base - prev > MAX_DISTANCE) {
				break;
			}

			while (
				(length != MAX_LENGTH)
				&& (static_cast<size_t>(prev) + length < input.size())
				&& (ip + length < in_end)
				&& (input[static_cast<size_t>(prev) + length] == ip[length])

				) {

				length++;
			}

			if (length > best_length) {
				best_length = length;
				best_distance = static_cast<uint16_t>(ip - in_base) - prev;
			}

			prev = chain[prev];
			chain_depth++;
		}

		if (best_length < MIN_MATCH) {
			ip++;
			continue;
		}

		uint8_t token = 0;

		uint32_t biased_match_length = best_length - MATCH_LENGTH_BIAS;
		uint32_t literal_length = ip - anchor;


		token |= (literal_length >= 15 ? 15 : literal_length) << 4;
		token |= (biased_match_length >= 15 ? 15 : biased_match_length);

		output.push_back(token);

		if (literal_length >= 15) {
			uint32_t extra = literal_length - 15;
			while (extra >= 255) {
				output.push_back(255);
				extra -= 255;
			}
			output.push_back(static_cast<uint8_t>(extra));
		}

		output.insert(output.end(), anchor, ip);
		
		output.insert(output.end(), reinterpret_cast<uint8_t*>(&best_distance), reinterpret_cast<uint8_t*>(&best_distance) + sizeof(best_distance));

		if (biased_match_length >= 15) {
			uint32_t extra_length = biased_match_length - 15;
			while (extra_length >= 255) {
				output.push_back(255);
				extra_length -= 255;
			}
			output.push_back(static_cast<uint8_t>(extra_length));
		}

		for (uint32_t k = 1; k < best_length; k++) {
			if (ip + k + 3 >= in_end) break;

			uint32_t h = hash(ip+k);
			chain[ip - in_base + k] = head[h];
			head[h] = ip - in_base + k;
		}

		ip += best_length;
		anchor = ip;
	}

	uint32_t literal_length = ip - anchor;


	uint8_t token = 0;
	token |= (literal_length >= 15 ? 15 : literal_length) << 4;
	output.push_back(token);


	if (literal_length >= 15) {

		uint32_t extra = literal_length - 15;
		while (extra >= 255) {
			output.push_back(255);
			extra -= 255;
		}
		output.push_back(static_cast<uint8_t>(extra));
	}

	output.insert(output.end(), anchor, ip);

	return output;

}

std::expected<std::vector<uint8_t>, lpz::Error>
lpz::lz77::decompress(std::span<const uint8_t> data) {

	if (data.size() >= std::numeric_limits<uint32_t>::max())
		return std::unexpected(Error{ ErrorCode::InputError, "LZ77 decompress: Input too large" });
	if (data.empty())
		return std::unexpected(Error{ ErrorCode::InputError, "LZ77 decompress: Empty Input" });

	std::vector<uint8_t> out;
	out.reserve(data.size() * 2);

	const uint8_t* ptr = data.data();
	const uint8_t* end = ptr + data.size();

	while (ptr < end) {

		uint8_t token = *ptr++;

		uint32_t literal_length = (token & 0xF0) >> 4;

		if (literal_length == 15) {
			uint8_t len_byte;
			do {
				len_byte = *ptr++;
				literal_length += len_byte;
			} while (len_byte == 255);
		}

		if (literal_length > 0) {
			out.insert(out.end(), ptr, ptr + literal_length);
			ptr += literal_length;
		}

		if (ptr >= end) break;

		uint32_t biased_match_length = token & 0x0F;

		uint16_t match_distance;
		memcpy(&match_distance, ptr, sizeof(match_distance));

		ptr += sizeof(match_distance);

		if (biased_match_length == 15) {
			uint8_t len_byte;
			do {
				len_byte = *ptr++;
				biased_match_length += len_byte;
			} while (len_byte == 255);
		}

		size_t match_start = out.size() - match_distance;

		uint32_t match_length = biased_match_length + MATCH_LENGTH_BIAS;

		for (uint32_t k = 0; k < match_length; ++k) {
			out.push_back(out[match_start + k]);
		}
	}

	return out;
}
