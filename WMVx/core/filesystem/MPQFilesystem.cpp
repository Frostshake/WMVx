#include "../../stdafx.h"
#include "MPQFileSystem.h"

#include <execution>
#include <algorithm>
#include <QFile>
#include <QDir>

namespace core {

	const std::vector<QString> MPQFileSystem::defaultMPQs = {
		"patch-3.MPQ",
		"patch-2.MPQ",
		"patch.MPQ",
		"alternate.MPQ",
		"expansion4.MPQ",
		"expansion3.MPQ",
		"expansion2.MPQ",
		"expansion1.MPQ",
		"lichking.MPQ",
		"expansion.MPQ",
		"world.MPQ",
		"world2.MPQ",
		"sound.MPQ",
		"art.MPQ",
		"common-3.MPQ",
		"common-2.MPQ",
		"common.MPQ",
		"interface.MPQ",
		"itemtexture.MPQ",
		"misc.MPQ",
		"model.MPQ",
		"texture.MPQ"
	};

	const std::vector<QString> MPQFileSystem::localeMPQs = {
		"patch-%s-3.MPQ",
		"patch-%s-2.MPQ",
		"patch-%s.MPQ",
		"expansion3-locale-%s.MPQ",
		"expansion2-locale-%s.MPQ",
		"expansion1-locale-%s.MPQ",
		"lichking-locale-%s.MPQ",
		"expansion-locale-%s.MPQ",
		"locale-%s.MPQ",
		"base-%s.MPQ"
	};


	uint64_t MPQFile::getFileSize() {
		return SFileGetFileSize(mpq_file, 0);
	}

	void MPQFile::read(void* dest, uint64_t bytes, uint64_t offset) {
		DWORD dwBytes = 1;
		SFileSetFilePointer(mpq_file, offset, NULL, FILE_BEGIN);
		SFileReadFile(mpq_file, dest, bytes, &dwBytes, NULL);
	}

	void MPQArchive::open(const QString& path) {
		if (!SFileOpenArchive((TCHAR*)path.toStdString().c_str(), 0, MPQ_OPEN_FORCE_MPQ_V1 | MPQ_OPEN_READ_ONLY, &mpq)) {
			int error = GetLastError();
			assert(false);
			//TODO error
		}
	}

	void MPQArchive::close() {
		if (mpq != nullptr) {
			SFileCloseArchive(mpq);
		}
	}

	MPQFile* MPQArchive::openFile(const QString& path)
	{
		auto file = new MPQFile();
		file->uri = path;

		if (!SFileOpenFileEx(mpq, path.toStdString().c_str(), 0, &file->mpq_file)) {
			int error = GetLastError();
			delete file;

			return nullptr;
		}

		return file;
	}

	void MPQArchive::closeFile(MPQFile* file)
	{
		if (file->mpq_file != nullptr) {
			SFileCloseFile(file->mpq_file);
		}

		delete file;
	}

	MPQFileSystem::MPQFileSystem(const QString& root, const QString& locale) :GameFileSystem(root, locale)
	{
		for (auto& name : defaultMPQs) {
			auto path = rootDirectory + QDir::separator() + name;
			if (QFile::exists(path)) {
				auto archive = std::make_unique<MPQArchive>();
				archive->open(path);
				mpqArchives.push_back({ name, std::move(archive) });
			}
		}

		for (auto name : localeMPQs) {
			auto path = rootDirectory + QDir::separator() + locale + QDir::separator() + name.replace("%s", locale);
			if (QFile::exists(path)) {
				auto archive = std::make_unique<MPQArchive>();
				archive->open(path);
				mpqArchives.push_back({ name, std::move(archive) });
			}
		}
	}

	MPQFileSystem::~MPQFileSystem()
	{
		for (auto& mpq : mpqArchives) {
			mpq.second->close();
		}
	}

	std::future<void> MPQFileSystem::load()
	{
		//eager load file list?
		return std::future<void>();
	}

	ArchiveFile* MPQFileSystem::openFile(const GameFileUri& uri)
	{
		if (uri.isPath()) {
			for (auto& mpq : mpqArchives) {
				auto temp = mpq.second->openFile(uri.getPath());
				if (temp != nullptr) {
					return temp;
				}
			}
		}

		return nullptr;
	}

	void MPQFileSystem::closeFile(ArchiveFile* file)
	{
		//TODO REMOVE THIS BACK HACK.
		//only works as the closing MPQ of the file doesnt need to be the owner.
		auto temp = MPQArchive();
		temp.closeFile((MPQFile*)file);
	}

	std::unique_ptr<std::list<GameFileUri::path_t>> MPQFileSystem::fileList()
	{
		std::unique_ptr<std::list<QString>> list_items = std::make_unique<std::list<QString>>();

		for (auto& mpq : mpqArchives) {
			auto list_file = mpq.second->openFile("(listfile)");
			if (list_file != nullptr) {

				auto list_file_size = list_file->getFileSize();
				auto list_file_buffer = std::vector<uint8_t>(list_file_size);
				list_file->read(list_file_buffer.data(), list_file_size);

				mpq.second->closeFile(list_file);

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