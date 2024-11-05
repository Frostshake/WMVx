#include "../../stdafx.h"
#include "MPQFileSystem.h"

#include <execution>
#include <algorithm>
#include <QFile>
#include <QDir>

namespace core {

	uint64_t MPQFile::getFileSize() {
		return _impl->size();
	}

	void MPQFile::read(void* dest, uint64_t bytes, uint64_t offset) {
		_impl->setPos(offset);
		_impl->read(dest, bytes);
	}

	MPQFileSystem::MPQFileSystem(const QString& root, const QString& locale) :GameFileSystem(root, locale)
	{
		auto discovered = WDBReader::Filesystem::discoverMPQArchives(root.toStdString());
		_impl = std::make_unique<WDBReader::Filesystem::MPQFilesystem>(root.toStdString(), std::move(discovered));
	}

	std::future<void> MPQFileSystem::load()
	{
		//eager load file list?
		return std::future<void>();
	}

	std::unique_ptr<ArchiveFile> MPQFileSystem::openFile(const GameFileUri& uri)
	{
		if (uri.isPath()) {
			auto raw = _impl->open(uri.getPath().toStdString());
			if (raw != nullptr) {
				return std::make_unique<MPQFile>(uri, std::move(raw));
			}
		}

		return nullptr;
	}

	std::unique_ptr<std::list<GameFileUri::path_t>> MPQFileSystem::fileList()
	{
		std::unique_ptr<std::list<QString>> list_items = std::make_unique<std::list<QString>>();

		const QString listfile_name = "(listfile)";
		for (auto& mpq : _impl->getHandles()) {
			HANDLE temp;
			if (SFileOpenFileEx(mpq.second, listfile_name.toStdString().c_str(), SFILE_OPEN_FROM_MPQ, &temp)) {
				auto source = std::make_unique<WDBReader::Filesystem::MPQFileSource>(temp);
				MPQFile list_file(listfile_name, std::move(source));

				auto list_file_size = list_file.getFileSize();
				if (list_file_size > 0) {
					auto list_file_buffer = std::vector<uint8_t>(list_file_size);
					list_file.read(list_file_buffer.data(), list_file_size);

					uint8_t* p = list_file_buffer.data();
					uint8_t* end = list_file_buffer.data() + list_file_size;

					while (p < end) {
						uint8_t* q = p;
						do {
							if (*q == '\r' || *q == '\n') // carriage return or new line
								break;
						} while (q++ <= end);

						QString line = QString::fromStdString(std::string((char*)p, q - p));

						p = q + 2;

						if (line.length() == 0) {
							break;
						}

						list_items->push_back(line);
					}
				}
			}
		}

		list_items->unique();

		return list_items;
	}

	GameFileUri MPQFileSystem::asFileId(const GameFileUri& uri)
	{
		return (GameFileUri::id_t)0;
	}

	GameFileUri MPQFileSystem::asFilePath(const GameFileUri& uri)
	{
		if (uri.isId()) {
			throw std::bad_variant_access();
		}

		return uri;
	}

	GameFileUri MPQFileSystem::asInternal(const GameFileUri& uri)
	{
		return asFilePath(uri);
	}

	GameFileUri MPQFileSystem::asInternal(const GameFileInfo& info)
	{
		return info.path;
	}

	GameFileInfo MPQFileSystem::asInfo(const GameFileUri& uri)
	{
		if (uri.isId()) {
			throw std::bad_variant_access();
		}

		auto info = GameFileInfo();
		info.id = 0;
		info.path = uri.getPath();
		return info;
	}

};