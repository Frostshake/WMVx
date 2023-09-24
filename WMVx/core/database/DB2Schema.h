#pragma once

#include <cstdint>
#include <vector>
#include <numeric>
#include "DB2Definitions.h"

namespace core {

	class DB2Field {
	public:
		enum class Type {
			INT,
			FLOAT,
			STRING,
			LANG_STRING,
			INLINE_STRING
		};

		const Type type;

		/// <summary>
		/// element size in bytes (per element)
		/// </summary>
		const uint8_t size;

		/// <summary>
		/// number of elements, arrays are count > 1
		/// </summary>
		const uint8_t count;

		const bool isRelation;

		const bool isInline;

		constexpr bool isArray() const {
			return count > 1;
		}

		constexpr uint16_t totalSize() const {
			return size * count;
		}

		constexpr static DB2Field id(uint8_t size) {
			return DB2Field(Type::INT, size, 1, false, false);
		}

		constexpr static DB2Field integer(uint8_t size) {
			return DB2Field(Type::INT, size);
		}

		constexpr static DB2Field integerArray(uint8_t totalSize, uint8_t count) {
			return DB2Field(Type::INT, totalSize / count, count);
		}

		constexpr static DB2Field floatingPoint(uint8_t size) {
			return DB2Field(Type::FLOAT, size);
		}

		constexpr static DB2Field floatingPointArray(uint8_t totalSize, uint8_t count) {
			return DB2Field(Type::FLOAT, totalSize / count, count);
		}

		constexpr static DB2Field stringRef() {
			return DB2Field(Type::STRING, sizeof(DB2StringRef));
		}

		constexpr static DB2Field langStringRef() {
			return DB2Field(Type::LANG_STRING, sizeof(DB2LangStringRef));
		}

		constexpr static DB2Field inlineString() {
			return DB2Field(Type::INLINE_STRING, 0);
		}

		constexpr static DB2Field relationship(uint8_t size) {
			return DB2Field(Type::INT, size, 1, true, false);
		}

	protected:

		constexpr DB2Field(Type t, uint8_t s, uint8_t c = 1, bool rel = false, bool inl = true)
			: type(t), size(s), count(c), isRelation(rel), isInline(inl)
		{}
	};

	template<class... T>
	class DB2Schema {
	public:
		constexpr DB2Schema(const T&&... init) : fields {{ init... }}
		{
		}

		const std::array<const DB2Field, sizeof...(T)> fields;

		constexpr uint32_t inlineFieldCount() const {
			return std::count_if(fields.cbegin(), fields.cend(), [](const DB2Field& field) constexpr {
				return field.isInline;
			});
		}

		constexpr uint32_t inlineStringCount() const {
			return std::count_if(fields.cbegin(), fields.cend(), [](const DB2Field& field) constexpr {
				return field.type == DB2Field::Type::INLINE_STRING;
			});
		}

		constexpr uint32_t recordSize() const {
			return std::accumulate(fields.cbegin(), fields.cend(), 
				(uint32_t)0, 
				[](uint32_t sum, const DB2Field& field) constexpr {
					return sum + (field.size * field.count);
				});
		}
	};
}