#include "lz77.h"
#include <cassert>

namespace {
	constexpr int MAX_DISTANCE = 128 * 1024;
	constexpr int MAX_LENGTH = 2 * 1024;
}

std::expected<std::vector<uint8_t>, lpz::Error> 
lpz::lz77::compress(std::span<const uint8_t> input) {

	if (input.size() >= std::numeric_limits<uint32_t>::max())
		return std::unexpected(Error{ ErrorCode::InputError, "LZ77 compress: Input too large" });
	if (input.empty())
		return std::unexpected(Error{ ErrorCode::InputError, "LZ77 compress: Empty Input" });
	
	std::vector<uint8_t> output;
	uint32_t in_pos = 0;
	uint32_t raw_counter = 0;

	while (in_pos != input.size()) {

		if (raw_counter == 255) {
			output.push_back(static_cast<uint8_t>(raw_counter));
			output.insert(output.end(), input.begin() + (in_pos - raw_counter), input.begin() + (in_pos));
			raw_counter = 0;
		}

		uint32_t best_length = 0;
		uint32_t best_distance = 0;

		uint32_t search_start = (in_pos > MAX_DISTANCE) ? in_pos - MAX_DISTANCE : 0;
		for (uint32_t i = search_start; i < in_pos; i++) {
			uint32_t length = 0;

			while (
				   (length != MAX_LENGTH)
				&& (i + length < input.size())
			    && (in_pos + length < input.size())
				&& (input[i + length] == input[in_pos + length])

				){

				length++;
			}

			if (length > best_length) {
				best_length = length;
				best_distance = in_pos - i;
			}
		}

		if (best_length < 8) {
			raw_counter++;
			in_pos++;
			continue;
		}

		if (raw_counter > 0) {
			assert(raw_counter <= 255);
			output.push_back(static_cast<uint8_t>(raw_counter));
			output.insert(output.end(), input.begin() + (in_pos - raw_counter), input.begin() + (in_pos));
			raw_counter = 0;
		}

		output.insert(output.end(), 0);
		output.insert(output.end(), reinterpret_cast<uint8_t*>(&best_length), reinterpret_cast<uint8_t*>(&best_length) + 4);
		output.insert(output.end(), reinterpret_cast<uint8_t*>(&best_distance), reinterpret_cast<uint8_t*>(&best_distance) + 4);

		in_pos += best_length;

	}

	if (raw_counter > 0) {
		assert(raw_counter <= 255);
		output.push_back(static_cast<uint8_t>(raw_counter));
		output.insert(output.end(), input.begin() + (in_pos  - raw_counter), input.begin() + (in_pos ));
	}

	return output;

}

std::expected<std::vector<uint8_t>, lpz::Error>
lpz::lz77::decompress(std::span<const uint8_t> data) {

	if (data.size() >= std::numeric_limits<uint32_t>::max())
		return std::unexpected(Error{ ErrorCode::InputError, "LZ77 decompress: Input too large" });
	if (data.empty())
		return std::unexpected(Error{ ErrorCode::InputError, "LZ77 decompress: Empty Input" });

	std::vector<uint8_t> out;

	for (uint32_t i = 0; i < data.size();) {

		if (data[i] != 0) {
			out.insert(out.end(), data.begin() + i+1, data.begin() + i + data[i]+1);
			i += data[i]+1;
			continue;
		}

		uint32_t length;
		uint32_t distance;
		memcpy(&length, &data[i]+1, 4);
		memcpy(&distance, &data[i]+1+4, 4);
		
		if (length == 0 || distance == 0) {
			return std::unexpected(Error{ ErrorCode::InputError, "invalid length/distance" });
		}

		for (unsigned int copy = 0; copy < length; copy++) {			
			out.push_back(out[out.size() - distance]);
		}

		i += 8+1;
	}

	return out;
}
