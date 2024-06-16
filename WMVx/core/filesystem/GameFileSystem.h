#pragma once

#include <QString>
#include <memory>
#include "GameFileUri.h"

namespace core {

	class ArchiveFile {
	public:
		virtual ~ArchiveFile() {}
		virtual uint64_t getFileSize() = 0;
		virtual void read(void* dest, uint64_t bytes, uint64_t offset = 0) = 0;
	protected:
		ArchiveFile(const GameFileUri& uri) : _uri(uri) {}
		GameFileUri _uri;
	};

	class GameFileSystem {
	public:
		GameFileSystem(const QString& root, const QString& locale) {
			rootDirectory = root;
		};

		GameFileSystem(GameFileSystem&&) = default;
		virtual ~GameFileSystem() {}

		virtual constexpr QChar seperator() const = 0;

		/// <summary>
		/// The initial load only guarantees that the filesystem is stable enough to work itself, and access the database files,
		///  once the returned future has finished, then the file system is fully loaded.
		/// </summary>
		virtual std::future<void> load() = 0;

		virtual std::unique_ptr<ArchiveFile> openFile(const GameFileUri& uri) = 0;
		virtual std::unique_ptr<std::list<GameFileUri::path_t>> fileList() = 0;

		// uri conversions:
		virtual GameFileUri asFileId(const GameFileUri& uri) = 0;
		virtual GameFileUri asFilePath(const GameFileUri& uri) = 0;
		// Convert to the filesystem perferred format.
		virtual GameFileUri asInternal(const GameFileUri& uri) = 0;
		virtual GameFileUri asInternal(const GameFileInfo& info) = 0;
		virtual GameFileInfo asInfo(const GameFileUri& uri) = 0;

	protected:
		QString rootDirectory;
	};

};