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

		/*
			Helpers for int and float types.
		*/
		template<typename T>
		constexpr static DB2Field value()
			requires std::is_floating_point_v<T> {
			return DB2Field::floatingPoint(sizeof(T));
		}

		template<typename T>
		constexpr static DB2Field value() 
			requires std::is_integral_v<T> {
			return DB2Field::integer(sizeof(T));
		}

		template<typename T>
		constexpr static DB2Field value()
			requires std::is_array_v<T>&& std::is_floating_point_v<std::remove_extent_t<T>> {
			return DB2Field::floatingPointArray(sizeof(T), std::extent<T>::value);
		}

		template<typename T>
		constexpr static DB2Field value()
			requires std::is_array_v<T>&& std::is_integral_v<std::remove_extent_t<T>> {
			return DB2Field::integerArray(sizeof(T), std::extent<T>::value);
		}


	protected:

		constexpr DB2Field(Type t, uint8_t s, uint8_t c = 1, bool rel = false, bool inl = true)
			: type(t), size(s), count(c), isRelation(rel), isInline(inl)
		{}
	};

#define DB2FieldId(var)				DB2Field::id(sizeof(var))
#define DB2FieldRelation(var)		DB2Field::relationship(sizeof(var))
#define DB2FieldValue(var)			DB2Field::value<decltype(var)>()
#define DB2FieldStringRef(var)		DB2Field::stringRef()
#define DB2FieldLangStringRef(var)	DB2Field::langStringRef()
#define DB2FieldinlineString(var)	DB2Field::inlineString()

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


	template<typename T>
	concept DB2RecordType = requires(T t) {
		typename T::Data;
		std::is_pod_v<typename T::Data>;
		std::is_same_v<decltype(t.data),typename T::Data>;
		std::is_integral_v<decltype(t.recordIndex)>;
		{ T::schema };
	};
}