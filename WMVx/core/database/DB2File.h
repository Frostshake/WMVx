#pragma once
#include <cstdint>
#include <vector>
#include <span>
#include "../filesystem/CascFileSystem.h"
#include "../utility/Exceptions.h"
#include "../utility/ScopeGuard.h"
#include "DB2Schema.h"
#include <execution>

namespace core {

	constexpr int DB2MagicSize = 4;

#pragma pack(push, 1)

	struct WDCSection_DB2Header //WDC3+
	{
		uint64_t tact_key_hash;          // TactKeyLookup hash
		uint32_t file_offset;            // absolute position to the beginning of the section
		uint32_t record_count;           // 'record_count' for the section
		uint32_t string_table_size;      // 'string_table_size' for the section
		uint32_t offset_records_end;     // Offset to the spot where the records end in a file with an offset map structure;
		uint32_t id_list_size;           // Size of the list of ids present in the section
		uint32_t relationship_data_size; // Size of the relationship data in the section
		uint32_t offset_map_id_count;    // Count of ids present in the offset map in the section
		uint32_t copy_table_count;       // Count of the number of deduplication entries (you can multiply by 8 to mimic the old 'copy_table_size' field)
	};

	enum WDCFieldCompression //WDC3+
	{
		// None -- usually the field is a 8-, 16-, 32-, or 64-bit integer in the record data. But can contain 96-bit value representing 3 floats as well
		field_compression_none,
		// Bitpacked -- the field is a bitpacked integer in the record data.  It
		// is field_size_bits long and starts at field_offset_bits.
		// A bitpacked value occupies
		//   (field_size_bits + (field_offset_bits & 7) + 7) / 8
		// bytes starting at byte
		//   field_offset_bits / 8
		// in the record data.  These bytes should be read as a little-endian value,
		// then the value is shifted to the right by (field_offset_bits & 7) and
		// masked with ((1ull << field_size_bits) - 1).
		field_compression_bitpacked,
		// Common data -- the field is assumed to be a default value, and exceptions
		// from that default value are stored in the corresponding section in
		// common_data as pairs of { uint32_t record_id; uint32_t value; }.
		field_compression_common_data,
		// Bitpacked indexed -- the field has a bitpacked index in the record data.
		// This index is used as an index into the corresponding section in
		// pallet_data.  The pallet_data section is an array of uint32_t, so the index
		// should be multiplied by 4 to obtain a byte offset.
		field_compression_bitpacked_indexed,
		// Bitpacked indexed array -- the field has a bitpacked index in the record
		// data.  This index is used as an index into the corresponding section in
		// pallet_data.  The pallet_data section is an array of uint32_t[array_count],
		//
		field_compression_bitpacked_indexed_array,
		// Same as field_compression_bitpacked
		field_compression_bitpacked_signed,
	};

	struct WDCFieldStorageInfo_DB2 //WDC3+
	{
		uint16_t field_offset_bits;
		uint16_t field_size_bits; // very important for reading bitpacked fields; size is the sum of all array pieces in bits - for example, uint32[3] will appear here as '96'
		// additional_data_size is the size in bytes of the corresponding section in
		// common_data or pallet_data.  These sections are in the same order as the
		// field_info, so to find the offset, add up the additional_data_size of any
		// previous fields which are stored in the same block (common_data or
		// pallet_data).
		uint32_t additional_data_size;
		uint32_t storage_type;

		union {
			struct {
				uint32_t bitOffset;
				uint32_t bitWidth;
				bool isSigned;
			} bitpacked;

			struct {
				uint32_t defaultValue;
			} commonData;

			struct {
				uint32_t bitOffset;
				uint32_t bitWidth;
				uint32_t arraySize;
			} pallet;

			struct {
				uint32_t val1;
				uint32_t val2;
				uint32_t val3;
			} raw;
		} compressionData;

	};

	struct WDCFieldStructure_DB2 //WDC3+
	{
		int16_t size;                   // size in bits as calculated by: byteSize = (32 - size) / 8; this value can be negative to indicate field sizes larger than 32-bits
		uint16_t position;              // position of the field within the record, relative to the start of the record
	};

	struct  WDCCopyTableEntry_DB2 //WDC3+
	{
		uint32_t id_of_new_row;
		uint32_t id_of_copied_row;
	};

	struct WDCOffsetMapEntry_DB2 //WDC3+
	{
		uint32_t offset;
		uint16_t size;
	};

	struct WDCRelationshipEntry_DB2 //WDC3+
	{
		uint32_t foreign_id;
		uint32_t record_index;
	};

	struct DB2_WDC3 {
		constexpr static const char* Magic = "WDC3";

		struct Header {
			uint8_t signature[4];
			uint32_t record_count;           // this is for all sections combined now
			uint32_t field_count;
			uint32_t record_size;
			uint32_t string_table_size;      // this is for all sections combined now
			uint32_t table_hash;             // hash of the table name
			uint32_t layout_hash;            // this is a hash field that changes only when the structure of the data changes
			uint32_t min_id;
			uint32_t max_id;
			uint32_t locale;                 // as seen in TextWowEnum
			uint16_t flags;                  // possible values are listed in Known Flag Meanings
			uint16_t id_index;               // this is the index of the field containing ID values; this is ignored if flags & 0x04 != 0
			uint32_t total_field_count;      // from WDC1 onwards, this value seems to always be the same as the 'field_count' value
			uint32_t bitpacked_data_offset;  // relative position in record where bitpacked data begins; not important for parsing the file
			uint32_t lookup_column_count;
			uint32_t field_storage_info_size;
			uint32_t common_data_size;
			uint32_t pallet_data_size;
			uint32_t section_count;          // new to WDC2, this is number of sections of data
		};

		using SectionHeader = WDCSection_DB2Header;
		using FieldStorageInfo = WDCFieldStorageInfo_DB2;
		using FieldStructure = WDCFieldStructure_DB2;
	};

	struct DB2_WDC5 {
		constexpr static const char* Magic = "WDC5";

		struct Header {
			uint8_t signature[4];
			uint32_t versionNum;
			uint8_t schemaString[128];
			uint32_t record_count;           // this is for all sections combined now
			uint32_t field_count;
			uint32_t record_size;
			uint32_t string_table_size;      // this is for all sections combined now
			uint32_t table_hash;             // hash of the table name
			uint32_t layout_hash;            // this is a hash field that changes only when the structure of the data changes
			uint32_t min_id;
			uint32_t max_id;
			uint32_t locale;                 // as seen in TextWowEnum
			uint16_t flags;                  // possible values are listed in Known Flag Meanings
			uint16_t id_index;               // this is the index of the field containing ID values; this is ignored if flags & 0x04 != 0
			uint32_t total_field_count;      // from WDC1 onwards, this value seems to always be the same as the 'field_count' value
			uint32_t bitpacked_data_offset;  // relative position in record where bitpacked data begins; not important for parsing the file
			uint32_t lookup_column_count;
			uint32_t field_storage_info_size;
			uint32_t common_data_size;
			uint32_t pallet_data_size;
			uint32_t section_count;          // new to WDC2, this is number of sections of data
		};

		using SectionHeader = WDCSection_DB2Header;
		using FieldStorageInfo = WDCFieldStorageInfo_DB2;
		using FieldStructure = WDCFieldStructure_DB2;
	};

#pragma pack(pop)


	struct WDCSectionView {
		std::span<uint8_t> data;

		std::span<uint8_t> records;
		std::span<uint8_t> strings;
		std::span<uint32_t> id_list;
		std::span<uint8_t> copy_table;
		std::span<uint8_t> offset_map;
		std::span<uint8_t> offset_map_id_list;
		std::span<uint8_t> relationship_map;

		std::span<WDCCopyTableEntry_DB2> copyTableEntries;
		std::span<WDCOffsetMapEntry_DB2> offsetMapEntries;
		std::span<WDCRelationshipEntry_DB2> relationshipEntries;
		std::span<uint32_t> offsetMapIdEntries;
	};

	template<typename T>
	struct WDCSection {
		WDCSectionView view;
		std::vector<T> records;
	};

	template<DB2RecordType T>
	class DB2File;

	template<typename R>
	class DB2Loader {
		friend class DB2File<R>;
	public:
		using Section = WDCSection<typename R>;

		virtual void open(ArchiveFile* file) = 0;

		virtual bool isSparse() const = 0;

		virtual const QString getString(uint32_t ref, const WDCSectionView* section_view, size_t record_index, size_t src_field_index) const = 0;

	protected:
		std::vector<Section> sections;
		std::span<Section> sections_view;
	};


	template<typename R, typename T>
	class DB2TypeLoader : public DB2Loader<R> {
	public:
		void open(ArchiveFile* file) override
		{
			assert(file);
			const QString fileName = "DB2 file"; //TODO real file name;
			const auto size = file->getFileSize();

			data.resize(size);
			file->read(data.data(), size);

			uint32_t offset = 0;

			std::memcpy(&header, data.data(), sizeof(T::Header));
			offset += sizeof(T::Header);

			std::string signature((char*)header.signature, sizeof(header.signature));

			if (signature != T::Magic) {
				throw BadSignatureException(fileName.toStdString(), signature, T::Magic);
			}

			const auto inline_count = R::schema.inlineFieldCount();
			if (inline_count != header.field_count) {
#ifdef _DEBUG
				// useful for debugging field issues.
				auto schema_fields = R::schema.fields;
#endif
				throw BadStructureException(fileName.toStdString(), "DB2 header doesnt match known schema field count.");
			}

			rawSections.resize(header.section_count);
			std::memcpy(rawSections.data(), data.data() + offset, header.section_count * sizeof(T::SectionHeader));

			offset += header.section_count * sizeof(T::SectionHeader);

			fields.resize(header.field_count);
			std::memcpy(fields.data(), data.data() + offset, header.field_count * sizeof(T::FieldStructure));
			offset += header.field_count * sizeof(T::FieldStructure);

			if (header.field_storage_info_size > 0) {
				assert(header.field_storage_info_size % sizeof(T::FieldStorageInfo) == 0);

				auto field_storage_count = header.field_storage_info_size / sizeof(T::FieldStorageInfo);
				fieldStorageInfo.resize(field_storage_count);

				std::memcpy(fieldStorageInfo.data(), data.data() + offset, header.field_storage_info_size);
				offset += header.field_storage_info_size;
			}

			indexedPalletData.resize(header.field_count);

			if (header.pallet_data_size > 0) {
				pallet_data = std::span<uint8_t>(data.data() + offset, header.pallet_data_size);
				offset += header.pallet_data_size;

				uint32_t pallet_offset = 0;
				for (uint32_t j = 0; j < header.field_count; j++) {
					const auto& fieldInfo = fieldStorageInfo[j];
					if (fieldInfo.storage_type == WDCFieldCompression::field_compression_bitpacked_indexed ||
						fieldInfo.storage_type == WDCFieldCompression::field_compression_bitpacked_indexed_array) {
						auto inner_size = fieldInfo.additional_data_size;
						indexedPalletData[j] = std::vector<uint32_t>(inner_size / 4);

						memcpy(indexedPalletData[j].data(), pallet_data.data() + pallet_offset, inner_size);
						pallet_offset += inner_size;
					}
				}
			}

			indexedCommonData.resize(header.field_count);

			if (header.common_data_size > 0) {
				common_data = std::span<uint8_t>(data.data() + offset, header.common_data_size);
				offset += header.common_data_size;

				uint32_t common_offset = 0;
				for (uint32_t j = 0; j < header.field_count; j++) {
					const auto& fieldInfo = fieldStorageInfo[j];
					if (fieldInfo.storage_type == WDCFieldCompression::field_compression_common_data) {
						indexedCommonData[j] = std::map<uint32_t, uint32_t>();

						for (uint32_t a = 0; a < (fieldInfo.additional_data_size / (4 + 4)); a++) {
							uint32_t left = 0;
							uint32_t right = 0;

							memcpy(&left, common_data.data() + common_offset, sizeof(uint32_t));
							common_offset += sizeof(uint32_t);
							memcpy(&right, common_data.data() + common_offset, sizeof(uint32_t));
							common_offset += sizeof(uint32_t);

							indexedCommonData[j][left] = right;
						}
					}
				}
			}

			if (fieldStorageInfo.size() != header.field_count) {
				throw BadStructureException(fileName.toStdString(), "DB2 field storage count doesnt match header count.");
			}

			for (const auto& rawSection : rawSections) {
				initSection(rawSection, isSparse());
			}

			if (header.section_count != rawSections.size() || rawSections.size() != this->sections.size()) {
				throw BadStructureException(fileName.toStdString(), "DB2 unexpected section count.");
			}

			if (isSparse()) {
				for (uint32_t i = 0; i < header.section_count; i++) {
					readSparseSection(rawSections[i], this->sections[i]);
				}
			}
			else {
				for (uint32_t i = 0; i < header.section_count; i++) {
					readNormalSection(rawSections[i], this->sections[i]);
				}
			}

			//now data has been read, free raw file data.
			indexedCommonData.clear();
			indexedCommonData.shrink_to_fit();

			indexedPalletData.clear();
			indexedPalletData.shrink_to_fit();

			this->sections_view = std::span(this->sections.begin(), this->sections.end());

		}

		bool isSparse() const {
			return (header.flags & 0x01) != 0;
		}

		const QString getString(uint32_t ref, const WDCSectionView* section_view, size_t record_index, size_t src_field_index) const override {
			assert(!isSparse());

			uint32_t field_bytes_offset = fieldStorageInfo[src_field_index].field_offset_bits / 8;
			//TODO Not sure which method is best?
			uint32_t alt_bytes_offset = 0;
			for (auto i = 0; i < src_field_index; i++) {
				alt_bytes_offset += R::schema.fields[i].totalSize();
			}

			assert(alt_bytes_offset == field_bytes_offset);

			uint64_t record_offset = (record_index * header.record_size);

			uint8_t* ptr = section_view->data.data() + record_offset;
			ptr += field_bytes_offset;
			ptr += ref;

			//TODO not sure what this means WMV had it? - seems to fix strings in multisection files
			ptr -= (header.record_count - rawSections[0].record_count) * header.record_size;

			return QString(reinterpret_cast<const char*>(ptr));
		}

	protected:

		inline bool hasSecondaryKeys() const {
			return (header.flags & 0x02) != 0;
		}

		void initSection(const typename T::SectionHeader& rawSection, bool is_sparse) {
			uint32_t section_offset = rawSection.file_offset;
			typename DB2Loader<R>::Section section;

			if (is_sparse) {
				section.view.records = std::span(data.data() + section_offset, 0);
				section.view.strings = std::span(data.data() + section_offset, 0);

				section_offset += (rawSection.offset_records_end - rawSection.file_offset);
			}
			else {
				section.view.records = std::span(data.data() + section_offset, rawSection.record_count * header.record_size);
				section_offset += rawSection.record_count * header.record_size;

				section.view.strings = std::span(data.data() + section_offset, rawSection.string_table_size);
				section_offset += rawSection.string_table_size;
			}

			section.view.id_list = std::span((uint32_t*)(data.data() + section_offset), rawSection.id_list_size / sizeof(uint32_t));
			section_offset += rawSection.id_list_size;

			section.view.copy_table = std::span(data.data() + section_offset, rawSection.copy_table_count * sizeof(WDCCopyTableEntry_DB2));
			section.view.copyTableEntries = std::span((WDCCopyTableEntry_DB2*)(data.data() + section_offset), rawSection.copy_table_count);
			section_offset += rawSection.copy_table_count * sizeof(WDCCopyTableEntry_DB2);

			//TODO Tidy - depending if 'hasSecondaryKeys', offsets are in different order.
			if (hasSecondaryKeys()) {
				//section.view.offset_map = std::span(data.data() + section_offset, rawSection.offset_map_id_count * sizeof(WDC3OffsetMapEntry_DB2));
				section.view.offsetMapEntries = std::span((WDCOffsetMapEntry_DB2*)(data.data() + section_offset), rawSection.offset_map_id_count);
				section_offset += rawSection.offset_map_id_count * sizeof(WDCOffsetMapEntry_DB2);

				//section.view.offset_map_id_list = std::span(data.data() + section_offset, rawSection.offset_map_id_count * sizeof(uint32_t));
				section.view.offsetMapIdEntries = std::span((uint32_t*)(data.data() + section_offset), rawSection.offset_map_id_count);
				section_offset += rawSection.offset_map_id_count * sizeof(uint32_t);

				if (rawSection.relationship_data_size > 0) {
					uint32_t relationship_count = *((uint32_t*)(data.data() + section_offset));
					// +4 min id
					// + 4 max id
					section.view.relationshipEntries = std::span((WDCRelationshipEntry_DB2*)(data.data() + section_offset + (sizeof(uint32_t) * 3)), relationship_count);

					uint32_t calc_size = (sizeof(uint32_t) * 3) + (relationship_count * sizeof(WDCRelationshipEntry_DB2));
					assert(rawSection.relationship_data_size >= calc_size);
				}

				section.view.relationship_map = std::span(data.data() + section_offset, rawSection.relationship_data_size * sizeof(WDCRelationshipEntry_DB2));
				section_offset += rawSection.relationship_data_size * sizeof(WDCRelationshipEntry_DB2);
			}
			else {
				//section.view.offset_map = std::span(data.data() + section_offset, rawSection.offset_map_id_count * sizeof(WDC3OffsetMapEntry_DB2));
				section.view.offsetMapEntries = std::span((WDCOffsetMapEntry_DB2*)(data.data() + section_offset), rawSection.offset_map_id_count);
				section_offset += rawSection.offset_map_id_count * sizeof(WDCOffsetMapEntry_DB2);

				if (rawSection.relationship_data_size > 0) {
					uint32_t relationship_count = *((uint32_t*)(data.data() + section_offset));
					// +4 min id
					// + 4 max id
					section.view.relationshipEntries = std::span((WDCRelationshipEntry_DB2*)(data.data() + section_offset + (sizeof(uint32_t) * 3)), relationship_count);

					uint32_t calc_size = (sizeof(uint32_t) * 3) + (relationship_count * sizeof(WDCRelationshipEntry_DB2));
					assert(rawSection.relationship_data_size >= calc_size);
				}

				section.view.relationship_map = std::span(data.data() + section_offset, rawSection.relationship_data_size * sizeof(WDCRelationshipEntry_DB2));
				section_offset += rawSection.relationship_data_size * sizeof(WDCRelationshipEntry_DB2);

				//section.view.offset_map_id_list = std::span(data.data() + section_offset, rawSection.offset_map_id_count * sizeof(uint32_t));
				section.view.offsetMapIdEntries = std::span((uint32_t*)(data.data() + section_offset), rawSection.offset_map_id_count);
				section_offset += rawSection.offset_map_id_count * sizeof(uint32_t);
			}


			section.view.data = std::span(section.view.records.data(), data.data() + section_offset);	//TODO check correct, should contain all data in the section.

			this->sections.push_back(section);

			//	//ensure offset is smaller than file size;
			//	// //TODO FIX !
			//	//TODO
		}
		
		void readSparseSection(const typename T::SectionHeader& rawSection, DB2Loader<R>::Section& section) {
			section.records.reserve(rawSection.record_count);
			auto record_inserter = std::back_inserter(section.records);

			//TODO offset map can be used to determine start and end of records ahead of time.
			uint32_t abs_section_pos_via_map = rawSection.file_offset;
			uint32_t abs_section_pos_via_sizeof = rawSection.file_offset;
			uint32_t section_data_offset = 0;
			for (uint32_t i = 0; i < rawSection.record_count; i++) {

				uint32_t start_offset = section_data_offset;

				auto offset_map_record = section.view.offsetMapEntries[i];
				std::span<uint8_t> record_view = std::span<uint8_t>(section.view.data.data() + (offset_map_record.offset - rawSection.file_offset), offset_map_record.size);


				if (record_view.size() <= 0) {	//TODO this shouldnt happen - check why
					continue;
				}

				//TODO DEBUG REMOVE
				//TODO THIS IS A HACK TO ENSURE THE RECORD STARTS IN THE CORRECT PLACE - INVESTIGATE WHY THIS IS NEEDED AND SOMTIMES THE PREVIOUS RECORD OVERRUNS / UNDERRUNS 
				section_data_offset = offset_map_record.offset - rawSection.file_offset;

				assert(abs_section_pos_via_map == abs_section_pos_via_sizeof);

				readSparseRecord(record_view, i, section.view, record_inserter);

				abs_section_pos_via_map += offset_map_record.size;
				abs_section_pos_via_sizeof += sizeof(R::Data);

				auto temp_diff = (offset_map_record.size - sizeof(R::Data));
				if (temp_diff != 0) {
					section_data_offset += temp_diff;
					abs_section_pos_via_sizeof += temp_diff;
				}

				//auto advancement = section_data_offset - start_offset;
				//assert(advancement == (offset_map_record.size - sizeof(uint32_t)));	// as the ID as extra				
			}
		}

		inline void readSparseRecord(const std::span<uint8_t>& record_view, uint32_t record_index, const WDCSectionView& section_view, std::back_insert_iterator<std::vector<R>>& record_inserter) {
			int32_t view_offset_bytes = 0;

			R record;
			record.recordIndex = record_index;
			std::span<uint8_t> record_fill_data = std::span((uint8_t*)&record.data, sizeof(record.data));
			uint32_t record_id = 0;

			auto schema_index = -1;
			auto record_offset = 0;

			if (section_view.id_list.size() > 0) {
				//if this uses external ids, force fill first row
				schema_index++;
				assert(R::schema.fields[schema_index].totalSize() == sizeof(uint32_t));
				uint32_t temp = section_view.id_list[record_index];
				record_id = temp;
				memcpy(record_fill_data.data(), &temp, sizeof(uint32_t));

				record_offset += sizeof(uint32_t);
			}

			constexpr auto has_inline_opt = requires(const R & t) {
				t.inlineStrings;
			};

			int32_t inline_string_index = 0;
			for (uint32_t j = 0; j < header.field_count; j++) {
				schema_index++;
				const auto& fieldInfo = fieldStorageInfo[j];
				const auto schemaField = R::schema.fields[schema_index];

				if (schemaField.type == DB2Field::Type::INLINE_STRING) {
					uint8_t* char_ptr = record_view.data() + view_offset_bytes;
					auto str = QString(reinterpret_cast<const char*>(char_ptr));

					view_offset_bytes += (str.length() + 1) * sizeof(uint8_t); //length + 1 (null)
					record_offset += 0;

					if constexpr (has_inline_opt) {
						record.inlineStrings[inline_string_index] = str;
					}

					inline_string_index++;
				}
				else {
					switch (fieldInfo.storage_type) {
					case WDCFieldCompression::field_compression_none:
					{
						assert(fieldInfo.field_offset_bits % 8 == 0);
						auto data_bytes = (fieldInfo.field_size_bits / 8);
						assert(data_bytes == schemaField.totalSize());
						memcpy(record_fill_data.data() + record_offset, record_view.data() + view_offset_bytes, data_bytes);

						record_offset += data_bytes;
						view_offset_bytes += data_bytes;
					}
					break;
					default:
						assert(false);
						break;
					}
				}
			}

			//TODO HANDLE RELATION FIELDS! 

			assert(record_offset == sizeof(R::Data));
			assert(record_offset == record_fill_data.size());

			record_inserter = record;
			cloneUsingCopyTable(record, section_view, record_inserter);
		}

		void readNormalSection(const typename T::SectionHeader& rawSection, DB2Loader<R>::Section& section) {

			//TODO handle
			if (rawSection.tact_key_hash != 0) {
				return;
			}

			section.records.reserve(rawSection.record_count);
			auto record_inserter = std::back_inserter(section.records);

			uint32_t section_offset = rawSection.file_offset;
			for (uint32_t i = 0; i < rawSection.record_count; i++) {
				std::span<uint8_t> record_view(data.data() + section_offset, header.record_size);
				section_offset += header.record_size;

				readNormalRecord(record_view, i, section.view, record_inserter);
			}
		}

		inline void readNormalRecord(const std::span<uint8_t>& record_view, uint32_t record_index, const WDCSectionView& section_view, std::back_insert_iterator<std::vector<R>>& record_inserter) {
			uint32_t view_offset_bits = 0;

			R record;
			record.recordIndex = record_index;
			std::span<uint8_t> record_fill_data = std::span((uint8_t*)&record.data, sizeof(record.data));
			uint32_t record_id = 0;

			auto schema_index = -1;
			auto record_offset = 0;

			if (section_view.id_list.size() > 0) {
				//if this uses external ids, force fill first row
				schema_index++;
				assert(R::schema.fields[schema_index].totalSize() == sizeof(uint32_t));
				uint32_t temp = section_view.id_list[record_index];
				record_id = temp;

				memcpy(record_fill_data.data(), &temp, sizeof(uint32_t));
				record_offset += sizeof(uint32_t);
			}

			//loop though all the fields, decode the value and put into record.

			for (uint32_t j = 0; j < header.field_count; j++) {
				schema_index++;
				std::vector<uint8_t> val;
				const auto& fieldInfo = fieldStorageInfo[j];
				const auto schemaField = R::schema.fields[schema_index];

				switch (fieldInfo.storage_type) {
				case WDCFieldCompression::field_compression_none:
				{
					assert(fieldInfo.field_offset_bits % 8 == 0);
					const auto field_bytes = fieldInfo.field_size_bits / 8;
					val.resize(field_bytes);
					memcpy(val.data(), record_view.data() + (view_offset_bits / 8), field_bytes);
				}
				break;

				case WDCFieldCompression::field_compression_bitpacked:
				{
					assert(fieldInfo.field_size_bits <= 32);
					uint32_t dest = readBitpackedValue(fieldInfo, record_view.data());
					val.resize(4);
					memcpy(val.data(), &dest, 4);
				}
				break;
				case WDCFieldCompression::field_compression_bitpacked_signed:
				{
					assert(fieldInfo.field_size_bits <= 32);
					uint32_t dest = readBitpackedValue(fieldInfo, record_view.data());
					uint32_t mask = uint32_t(1) << (fieldInfo.compressionData.bitpacked.bitWidth - 1);
					dest = (dest ^ mask) - mask;
					val.resize(4);
					memcpy(val.data(), &dest, 4);
				}

				break;
				case WDCFieldCompression::field_compression_common_data:
				{
					val.resize(4);
					auto map_it = indexedCommonData[j].find(record_id);
					uint32_t temp_val = fieldInfo.compressionData.commonData.defaultValue;
					if (map_it != indexedCommonData[j].end()) {
						temp_val = map_it->second;

					}
					memcpy(val.data(), &temp_val, sizeof(uint32_t));
				}
				break;
				case WDCFieldCompression::field_compression_bitpacked_indexed:
				{
					assert(fieldInfo.field_size_bits <= 32);
					uint32_t dest = readBitpackedValue(fieldInfo, record_view.data());
					val.resize(4);
					memcpy(val.data(), &indexedPalletData[j][dest], 4);
				}
				break;
				case WDCFieldCompression::field_compression_bitpacked_indexed_array:
				{
					std::vector<uint32_t> temp(fieldInfo.compressionData.pallet.arraySize);
					for (uint32_t k = 0; k < fieldInfo.compressionData.pallet.arraySize; k++) {
						//	
						assert(fieldInfo.field_size_bits <= 32);
						assert(fieldInfo.compressionData.pallet.bitWidth <= 32);
						uint32_t dest = readBitpackedValue(fieldInfo, record_view.data());
						auto key = (dest * fieldInfo.compressionData.pallet.arraySize) + k;	//TODO not sure if key calculation is correct (see wow export / WMV)
						temp[k] = indexedPalletData[j][key];
					}

					val.resize(temp.size() * 4);
					memcpy(val.data(), temp.data(), temp.size() * 4);
				}
				break;
				default:
					assert(false);
					break;
				}

				//shrink if needed.
				if (fieldInfo.storage_type != WDCFieldCompression::field_compression_none) {
					if (val.size() > schemaField.totalSize()) {
						val.resize(schemaField.totalSize(), 0);
					}
				}

				assert(val.size() == schemaField.totalSize());

				// if the id hasnt been specified via the external view, assume it is the first field and int32 size.
				// (this might not always be correct?) - TODO THIS ISNT CORRECT! - not reliable.
				if (record_id == 0 && j == 0) {
					if (sizeof(record_id) == schemaField.totalSize()) {
						memcpy(&record_id, val.data(), schemaField.totalSize());
					}
				}

				for (auto item : val) {
					record_fill_data[record_offset] = item;
					record_offset += sizeof(item);
				}

				view_offset_bits += fieldInfo.field_size_bits;
			}


			//TODO DF relations *can* be mid record, also if hasSecondaryKeys is set, relation lookup needs to be done via the record ID, not the index.

			//TODO do relations ever appear in the middle of a record or are they always the end? also handle multiple relations
			// currently this just force-fills relations assumed to be at the end.
			const auto schema_max_index = R::schema.fields.size() - 1;

			if (schema_index < schema_max_index) {
				schema_index++;	//adance to the next target record.

				auto relation_index = 0;
				for (auto j = schema_index; j <= schema_max_index; j++, schema_index++) {

					const auto schemaField = R::schema.fields[schema_index];
					if (schemaField.isRelation) {
						assert(relation_index == 0);	//TODO only currently handling 1 relation.

						uint32_t foreign_id = 0;

						auto find_result = std::find_if(std::execution::par,
							section_view.relationshipEntries.begin(),
							section_view.relationshipEntries.end(),
							[record_index](const WDCRelationshipEntry_DB2& rel_record) {
								return rel_record.record_index == record_index;
							}
						);

						if (find_result != section_view.relationshipEntries.end()) {
							foreign_id = find_result->foreign_id;
						}

						std::array<uint8_t, sizeof(uint32_t)> val;
						memcpy(val.data(), &foreign_id, sizeof(uint32_t));
						for (auto item : val) {
							record_fill_data[record_offset] = item;
							record_offset += sizeof(item);
						}

						relation_index++;
					}
				}
			}

			assert(record_offset == sizeof(R::Data));
			assert(record_offset == record_fill_data.size());

			record_inserter = record;
			cloneUsingCopyTable(record, section_view, record_inserter);
		}

		void cloneUsingCopyTable(const R& src, const WDCSectionView& section_view, std::back_insert_iterator<std::vector<R>>& record_inserter) {

			constexpr auto has_id_field = requires(const R & t) {
				t.data.id;
			};

			if constexpr (has_id_field) {
				if (section_view.copyTableEntries.size() > 0) {
					std::mutex mut;
					std::for_each(std::execution::par, section_view.copyTableEntries.begin(), section_view.copyTableEntries.end(), [src, &mut, &record_inserter](const WDCCopyTableEntry_DB2& entry) {
						if (entry.id_of_copied_row == src.data.id) {
							auto cloned_record = src;
							cloned_record.data.id = entry.id_of_new_row;
							std::scoped_lock lock(mut);
							record_inserter = cloned_record;
						}
						});
				}
			}
		}

		uint32_t readBitpackedValue(const typename T::FieldStorageInfo& info, uint8_t* data_ptr) {
			const uint32_t size_view_bytes = (info.field_size_bits + (info.field_offset_bits & 7) + 7) / 8;
			const uint32_t offset = info.field_offset_bits / 8;

			assert(size_view_bytes <= sizeof(uint64_t));
			assert(info.field_size_bits <= 32);

			const uint32_t bitWidth = info.field_size_bits;
			const uint32_t bitOffset = info.field_offset_bits;

			const auto bitsToRead = bitOffset & 7;
			uint32_t result = *reinterpret_cast<uint64_t const*>(data_ptr + offset) << (64 - bitsToRead - bitWidth) >> (64 - bitWidth);

			return result;
		}


		T::Header header;

		std::vector<uint8_t> data;
		std::span<uint8_t> pallet_data;
		std::span<uint8_t> common_data;

		std::vector<typename T::SectionHeader> rawSections;
		std::vector<typename T::FieldStorageInfo> fieldStorageInfo;
		std::vector<typename T::FieldStructure> fields;

		std::vector<std::vector<uint32_t>> indexedPalletData;
		std::vector<std::map<uint32_t, uint32_t>> indexedCommonData;

	};


	template<DB2RecordType T>
	class DB2File
	{
	public:
		using SectionView = WDCSectionView;
		using Section = WDCSection<typename T>;

		/* stl - like iterators for reading. */
		struct ConstIterator {
		public:
			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = const T;
			using pointer = const T*;
			using reference = const T&;
			using section_reference = const SectionView&;
			using section_iter = std::vector<Section>::const_iterator;
			using record_iter = std::vector<T>::const_iterator;
			using owner = const std::vector<Section>&;

	
			ConstIterator(owner owner, section_iter&& section)
				:	m_owner(owner),
				m_target_section(section)
			{
				if (m_target_section == m_owner.cbegin()) {
					initNextSection();
				}
				else if (m_target_section == m_owner.cend()) {
					//nothing can be done here.
				}
				else {
					throw std::logic_error("Cannot create db2 iterator.");
				}
			}

			reference operator*() const { 
				return *m_record_target;
			}

			pointer operator->() { 
				return &(*m_record_target);
			}

			section_reference section() const {
				return m_target_section->view;
			}

			ConstIterator& operator++() { 
				if (++m_record_target == m_record_end) {
					if (++m_target_section != m_owner.cend()) {
						initNextSection();
					}
				}
				
				return *this; 
			}

			ConstIterator operator++(int) {
				ConstIterator it = *this;
				++(*this);
				return it;
			}

			friend bool operator== (const ConstIterator& a, const ConstIterator& b) {
				if (a.m_target_section == b.m_target_section) {
					const bool a_valid = a.recordsValid();
					const bool b_valid = b.recordsValid();

					if (a_valid && b_valid) {
						return a.m_record_target == b.m_record_target;
					}

					return a_valid == b_valid;
				}

				return false;
			};
			friend bool operator!= (const ConstIterator& a, const ConstIterator& b) {
				if (a.m_target_section == b.m_target_section) {
					const bool a_valid = a.recordsValid();
					const bool b_valid = b.recordsValid();

					if (a_valid && a_valid) {
						return a.m_record_target != b.m_record_target;
					}

					return a_valid != b_valid;
				}
				

				return true;
			};

		private:

			constexpr bool recordsValid() const {
				return m_target_section != m_owner.cend();
			}

			inline void initNextSection() {

				//automatically skip empty sections.
				while (m_target_section != m_owner.cend() && m_target_section->records.empty()) {
					++m_target_section;
				}

				if (m_target_section != m_owner.end()) {
					m_record_begin = m_target_section->records.begin();
					m_record_target = m_record_begin;
					m_record_end = m_target_section->records.end();
				}
			}

			owner m_owner;
			section_iter m_target_section;
			record_iter m_record_begin;
			record_iter m_record_target;
			record_iter m_record_end;
		};

		ConstIterator cbegin() const {
			return ConstIterator(loader->sections, loader->sections.cbegin());
		}

		ConstIterator cend() const {
			return ConstIterator(loader->sections, loader->sections.cend());
		}
	

		DB2File(const QString& name) : fileName(name) {}
		DB2File(DB2File<T>&&) = default;
		virtual ~DB2File() {}

		void open(CascFileSystem* fs) {

			static_assert(T::schema.recordSize() == sizeof(T::Data), "Schema size does not match data size.");

			std::unique_ptr<ArchiveFile> file = fs->openFile(fileName);
			if (file == nullptr) {
				throw FileIOException(fileName.toStdString(), "db file doesnt exist.");
			}

			const auto size = file->getFileSize();

			if (size < DB2MagicSize) {
				throw BadSignatureException("DB2 header too small.");
			}

			std::string signature(DB2MagicSize, '\0');
			file->read(signature.data(), DB2MagicSize);

			if (signature == DB2_WDC3::Magic) {
				loader = std::make_unique<DB2TypeLoader<T, DB2_WDC3>>();
			}
			else if (signature == DB2_WDC5::Magic) {
				loader = std::make_unique < DB2TypeLoader<T, DB2_WDC5>>();
			}
			else {
				throw BadSignatureException(fileName.toStdString(), signature, "WDC*");
			}

			loader->open(file.get());
		}

		const QString getString(uint32_t ref, const SectionView* section_view, size_t record_index, size_t src_field_index) const {
			return loader->getString(ref, section_view, record_index, src_field_index);
		}

		inline const std::span<Section>& getSections() const {
			return loader->sections_view;
		}

		inline bool isSparse() const {
			return loader->isSparse();
		}

	private:

		const QString fileName;
		std::unique_ptr<DB2Loader<T>> loader;
	};

};