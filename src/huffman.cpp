#include "huffman.h"
#include <array>
#include <algorithm>
#include <cassert>

namespace {

	constexpr int MAX_BITS = 16;

	std::array<uint32_t, 256> create_histogram(std::span<const uint8_t> data) {

		std::array<uint32_t, 256> f = {};

		for (uint8_t value : data) {
			f[value]++;
		}
		return f;
	}

	uint32_t reverse_bits(uint32_t v, int n) {
		uint32_t r = 0;
		for (int i = 0; i < n; i++) {
			if (v & (1 << i)) {
				r |= (1 << (n - 1 - i));
			}
		}
		return r;
	}

	std::array<uint32_t, 256> lengths_to_codes(std::span<const uint8_t> lengths) {

		std::array<int, MAX_BITS+1> codes_for_length = {};

		for (int i = 0; i < 256; i++) {
			codes_for_length[lengths[i]]++;
		}

		std::array<uint32_t, MAX_BITS+1> next_code = {};
		uint32_t code = 0;

		for (int len = 1; len <= MAX_BITS; len++) {
			code = (code + codes_for_length[len - 1]) << 1;
			next_code[len] = code;
		}

		std::array<uint32_t, 256> result_codes = {};

		for (int i = 0; i < 256; i++) {
			int len = lengths[i];
			if (len == 0) continue;

			uint32_t canonical_val = next_code[len];

			next_code[len]++;

			result_codes[i] = reverse_bits(canonical_val, len);
		}

		return result_codes;
	}
}

namespace lpz::huffman {

	std::vector<uint8_t> compress(std::span<const uint8_t> data) {

		struct Package {
			uint32_t weight;
			int original_index; // -1 = merged node
		};

		std::array<uint32_t,256> histogram = create_histogram(data);

		std::vector<Package> leaves;
		for (int i = 0; i < 256; i++) {
			if (histogram[i] > 0) {
				leaves.push_back({ histogram[i], i });
			}
		}
			
		auto sort_packages = [](const Package& a, const Package& b) 
		{
			if (a.weight != b.weight)
				return a.weight < b.weight;
			// Prefer Packages over Leaves
			return a.original_index < b.original_index; 
			};


		std::sort(leaves.begin(), leaves.end(), sort_packages);

		std::array<uint8_t, 256> lengths = {};
			
		int n = leaves.size();

		std::vector<std::vector<Package>> levels(MAX_BITS);
		levels[0] = leaves;

		for (int i = 0; i < MAX_BITS - 1; ++i) {
			std::vector<Package> new_packages;
			const auto& prev_level = levels[i];
			for (size_t k = 0; k + 1 < prev_level.size(); k += 2) {
				uint32_t sum_weight = prev_level[k].weight + prev_level[k + 1].weight;
				new_packages.push_back({ sum_weight, -1 });
			}

			// Merge new packages with the leaves
			levels[i + 1].resize(leaves.size() + new_packages.size());
			std::merge(
				leaves.begin(), leaves.end(),          
				new_packages.begin(), new_packages.end(), 
				levels[i + 1].begin(),                 
				sort_packages
			);
		}

		size_t items_needed = 2 * n - 2;

		for (int i = MAX_BITS - 1; i >= 0; --i) {
			size_t next_items_needed = 0;

			for (size_t k = 0; k < items_needed; ++k) {
				const auto& item = levels[i][k];

				if (item.original_index != -1) {
					lengths[item.original_index]++;
				}
				else {
					next_items_needed += 2;
				}
			}

			items_needed = next_items_needed;
		}
		
		auto canonical_codes = lengths_to_codes(lengths);

		std::vector<uint8_t> coded_bytes;
		coded_bytes.reserve(data.size() * 0.65); 

		uint32_t uncompsize = data.size();
		const uint8_t* size_ptr = reinterpret_cast<const uint8_t*>(&uncompsize);
		coded_bytes.insert(coded_bytes.end(), lengths.cbegin(), lengths.cend());
		coded_bytes.insert(coded_bytes.end(), size_ptr, size_ptr + sizeof(uint32_t));

		uint64_t bit_buff = 0; 
		int buff_size = 0;

		for (uint8_t val : data) {
			uint32_t code = canonical_codes[val];
			int len = lengths[val];

			bit_buff |= (uint64_t)code << buff_size;
			buff_size += len;

			while (buff_size >= 8) {
				coded_bytes.push_back(bit_buff & 0xFF); 
				bit_buff >>= 8;             
				buff_size -= 8;                         
			}
		}

		if (buff_size > 0) {
			coded_bytes.push_back(bit_buff & 0xFF);
		}

		return coded_bytes;
	}

	std::vector<uint8_t> decompress(std::span<const uint8_t> data) {

		assert(data.size() > 256);
		
		constexpr int TABLE_BITS = 16;
		constexpr int TABLE_SIZE = 1 << TABLE_BITS;

		struct Entry {
			uint8_t symbol;
			uint8_t length;  
		};

		auto canonical_codes = lengths_to_codes(data.subspan(0, 256));

		size_t out_size = 0;
		memcpy(&out_size, data.data() + 256, 4);

		std::vector<Entry> table(TABLE_SIZE);
		for (auto& e : table) e.length = 0;

		for (int s = 0; s < 256; s++) {
			int len = data[s];  
			if (len == 0) continue;

			uint32_t code = canonical_codes[s];
			int fill = 1 << (TABLE_BITS - len);
			for (int i = 0; i < fill; i++) {
				int index = code | (i << len);
				table[index] = { (uint8_t)s, (uint8_t)len };
			}
		}

		uint64_t bitbuf = 0;
		int bits_in_buf = 0;
		size_t pos = 256+4;
		size_t byte_size = data.size();

		auto refill = [&]() 
		{
			while (bits_in_buf <= 56 && pos < byte_size) {
				bitbuf |= uint64_t(data[pos++]) << bits_in_buf;
				bits_in_buf += 8;
			}
		};

		std::vector<uint8_t> decoded;

		while (decoded.size() < out_size) {
			if (bits_in_buf < TABLE_BITS) {
				if (pos < byte_size) {
					refill();
				}
			}

			Entry e = table[bitbuf & (TABLE_SIZE - 1)];

			decoded.push_back(e.symbol);
			bitbuf >>= e.length;
			bits_in_buf -= e.length;
		}

		return decoded;
	}
}