#pragma once
#include <cstdint>
#include <vector>
#include <span>
#include "../filesystem/MPQFileSystem.h"
#include "../utility/Exceptions.h"

namespace core {
	struct DBCHeader {
		uint8_t signature[4];
		uint32_t recordCount;
		uint32_t fieldCount;
		uint32_t recordSize;
		uint32_t stringBlockSize;
	};

	template<typename T>
	class DBCFile
	{
	public:
		DBCFile(const QString name) : fileName(name) {}
		DBCFile(DBCFile<T>&&) = default;
		virtual ~DBCFile() {}

		void open(MPQFileSystem* fs) {
			ArchiveFile* file = fs->openFile(fileName);
			auto size = file->getFileSize();
			data.resize(size);
			file->read(data.data(), size);
			fs->closeFile(file);

			std::memcpy(&header, data.data(), sizeof(DBCHeader));
			std::string signature((char*)header.signature, sizeof(header.signature));

			if (signature != "WDBC") {
				throw BadSignatureException(fileName.toStdString(), signature, "WDBC");
			}

			const int own_record_size = sizeof(T);
			if (header.fieldCount * 4 != header.recordSize || header.fieldCount * 4 != own_record_size) {
				throw BadStructureException(fileName.toStdString(), "Field count size");
			}

			records = std::span(reinterpret_cast<T*>(data.data() + sizeof(DBCHeader)), header.recordCount);
			strings = std::span(data.data() + sizeof(DBCHeader) + (header.recordSize * header.recordCount), header.stringBlockSize);
		}

		const DBCHeader& getHeader() const {
			return header;
		}

		const QString& getFileName() const {
			return fileName;
		}

		const std::span<T>& getRecords() const {
			return records;
		}

		const QString getString(uint32_t ref) const {
			return QString(reinterpret_cast<const char*>(strings.data() + ref));
		}

	private:
		DBCHeader header;
		const QString fileName;

		std::vector<uint8_t> data;
		std::span<T> records;
		std::span<uint8_t> strings;
	};

};