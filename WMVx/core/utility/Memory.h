#pragma once

#include <array>
#include <memory>
#include <stdexcept>
#include <span>
#include <vector>

namespace core {
	/// <summary>
	/// An attempt at making a 'safer' way of copying raw memory.
	/// </summary>
	template<typename Td, typename Ts>
	inline void memcpy_x(const std::vector<Td>& dest, std::span<Ts> src, size_t src_byte_offset, size_t src_byte_count) {

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

	template<typename Td, typename Ts>
	inline void memcpy_x(const std::vector<Td>& dest, const std::vector<Ts>& src, size_t src_byte_offset, size_t src_byte_count) {
		memcpy_x(dest, std::span(src), src_byte_offset, src_byte_count);
	}

	template<typename T, size_t Capacity>
	class StackVector {
	public:
		using storage_type = std::array<T, Capacity>;
		using value_type = storage_type::value_type;
		using size_type = storage_type::size_type;
		using difference_type = storage_type::difference_type;
		using reference = storage_type::reference;
		using const_reference = storage_type::const_reference;
		using pointer = storage_type::pointer;
		using const_pointer = storage_type::const_pointer;
		using iterator = storage_type::iterator;
		using const_iterator = storage_type::const_iterator;

		StackVector() : _size(0) {}
		StackVector(std::initializer_list<T> init) : _size(0) {
			if (init.size() > Capacity) {
				throw std::length_error("Capacity exceeded.");
			}
			size_t i = 0;
			for (const auto& val : init) {
				_data[i++] = val;
			}

			_size = init.size();
		}

		size_type size() const {
			return _size;
		}

		constexpr size_type capacity() const {
			return Capacity;
		}

		bool empty() const {
			return _size == 0;
		}

		void clear() {
			_size = 0;
		}

		reference at(size_type pos) {
			if (pos >= _size) {
				throw std::out_of_range("Out of range.");
			}

			return _data[pos];
		}

		const_reference at(size_type pos) const {
			if (pos >= _size) {
				throw std::out_of_range("Out of range.");
			}

			return _data[pos];
		}

		reference operator[](size_type pos) {
			assert(pos < _size);
			return _data[pos];
		}

		const_reference operator[](size_type pos) const {
			assert(pos < _size);
			return _data[pos];
		}

		reference front() {
			assert(_size > 0);
			return _data[0];
		}

		const_reference front() const {
			assert(_size > 0);
			return _data[0];
		}

		reference back() {
			assert(_size > 0);
			return _data[_size - 1];
		}

		const_reference back() const {
			assert(_size > 0);
			return _data[_size - 1];
		}

		pointer data() noexcept {
			return _data.data();
		}

		const_pointer data() const noexcept {
			return _data.data();
		}

		iterator begin() noexcept {
			return iterator(_data.data(), 0);
		}

		const_iterator begin() const noexcept {
			return const_iterator(_data.data(), 0);
		}

		const_iterator cbegin() const noexcept {
			return const_iterator(_data.data(), 0);
		}

		iterator end() noexcept {
			return iterator(_data.data(), _size);
		}

		const_iterator end() const noexcept {
			return const_iterator(_data.data(), _size);
		}

		const_iterator cend() const noexcept {
			return const_iterator(_data.data(), _size);
		}

		void push_back(const value_type& value) {
			if (_size == Capacity) {
				throw std::length_error("Capacity exceeded.");
			}

			_data[_size++] = value;

		}

		void push_back(value_type&& value) {
			if (_size == Capacity) {
				throw std::length_error("Capacity exceeded.");
			}

			_data[_size++] = std::forward<value_type>(value);
		}

		void pop_back() {
			assert(_size > 0);
			_size--;
		}

	private:
		size_type _size;
		storage_type _data;
	};

};