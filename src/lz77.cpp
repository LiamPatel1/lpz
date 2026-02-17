#include "lz77.h"
#include <cassert>
#include <iostream>

namespace {

	constexpr uint16_t MAX_DISTANCE = 65535;
	constexpr uint32_t MAX_LENGTH = 2 * 1024;

	constexpr int MAX_CHAIN = 32;

	constexpr int MIN_MATCH = 5;
	constexpr int MATCH_LENGTH_BIAS = MIN_MATCH;

	constexpr uint32_t HASH_BITS = 15;
	constexpr uint32_t HASH_SIZE = 1 << HASH_BITS;

	static_assert(MIN_MATCH >= MATCH_LENGTH_BIAS);

	inline uint32_t hash(const uint8_t* p) {
		return ((p[0] << 8) ^ (p[1] << 4) ^ p[2]) & ((1u << HASH_BITS) - 1);
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
	size_t in_pos = 0;
	uint32_t raw_counter = 0;

	while (in_pos < input.size()) {

		if (in_pos + std::max(3, MIN_MATCH) >= input.size()) {
			raw_counter++;
			in_pos++;
			continue;
		}

		uint32_t best_length = 0;
		uint16_t best_distance = 0;

		uint32_t next_hash = hash(&input[in_pos]);

		uint32_t prev = head[next_hash];
		head[next_hash] = static_cast<uint32_t>(in_pos);

		chain[in_pos] = prev;

		int chain_depth = 0;

		while (prev != -1 && chain_depth < MAX_CHAIN) {
	
			uint32_t length = 0;

			if (in_pos - prev > MAX_DISTANCE) {
				break;
			}

			while (
				(length != MAX_LENGTH)
				&& (static_cast<size_t>(prev) + length < input.size())
				&& (in_pos + length < input.size())
				&& (input[static_cast<size_t>(prev) + length] == input[in_pos + length])

				) {

				length++;
			}

			if (length > best_length) {
				best_length = length;
				best_distance = static_cast<uint16_t>(in_pos) - prev;
			}

			prev = chain[prev];
			chain_depth++;
		}

		if (best_length < MIN_MATCH) {
			raw_counter++;
			in_pos++;
			continue;
		}

		uint8_t token = 0;

		uint32_t biased_length = best_length - MATCH_LENGTH_BIAS;

		token |= (raw_counter >= 15 ? 15 : raw_counter) << 4;
		token |= (biased_length >= 15 ? 15 : biased_length);

		output.push_back(token);

		size_t literal_start = in_pos - raw_counter;

		if (raw_counter >= 15) {
			uint32_t extra = raw_counter - 15;
			while (extra >= 255) {
				output.push_back(255);
				extra -= 255;
			}
			output.push_back(static_cast<uint8_t>(extra));
		}

		output.insert(output.end(), input.data() + literal_start, input.data() + in_pos);
		raw_counter = 0;
		
		output.insert(output.end(), reinterpret_cast<uint8_t*>(&best_distance), reinterpret_cast<uint8_t*>(&best_distance) + sizeof(best_distance));

		if (biased_length >= 15) {
			uint32_t extra_length = biased_length - 15;
			while (extra_length >= 255) {
				output.push_back(255);
				extra_length -= 255;
			}
			output.push_back(static_cast<uint8_t>(extra_length));
		}

		for (uint32_t k = 1; k < best_length; k++) {
			if (in_pos + k + 3 >= input.size()) break;

			uint32_t h = hash(&input[in_pos + k]);
			chain[in_pos + k] = head[h];
			head[h] = static_cast<uint32_t>(in_pos) + k;
		}

		in_pos += best_length;
	}

	size_t literal_start = in_pos - raw_counter;  

	uint8_t token = 0;
	token |= (raw_counter >= 15 ? 15 : raw_counter) << 4;
	output.push_back(token);

	if (raw_counter >= 15) {
		uint32_t extra = raw_counter - 15;
		while (extra >= 255) {
			output.push_back(255);
			extra -= 255;
		}
		output.push_back(static_cast<uint8_t>(extra));
	}

	output.insert(output.end(), input.data() + literal_start, input.data() + in_pos);

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
