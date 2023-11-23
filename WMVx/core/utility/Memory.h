#pragma once

#include <stdexcept>
#include <vector>

namespace core {
	/// <summary>
	/// An attempt at making a 'safer' way of copying raw memory.
	/// </summary>
	template<typename Td, typename Ts>
	inline void memcpy_x(const std::vector<Td>& dest, const std::vector<Ts>& src, size_t src_byte_offset, size_t src_byte_count) {

		const auto dest_size_bytes = sizeof(Td) * dest.size();
		const auto src_size_bytes = sizeof(Ts) * src.size();
		const auto src_copy_bytes = src_size_bytes - src_byte_offset;
		if (src_byte_count > src_copy_bytes) {
			// the requested source is larger than what is available / extends beyond end of source.
			throw std::runtime_error("Source memory error.");
		}

		const auto result = memcpy_s(
			(void*)dest.data(), 
			dest_size_bytes, 
			(void*)(src.data() + src_byte_offset), 
			src_byte_count);

		if (result != 0) {
			throw std::runtime_error("Destination memory error.");
		}
	}
};