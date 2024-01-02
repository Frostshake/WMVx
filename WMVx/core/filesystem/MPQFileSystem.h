#pragma once

#include <deque>
#include <vector>
#include "GameFileSystem.h"

namespace core {

	class MPQFile : public ArchiveFile {
	public:
		uint64_t getFileSize() override;
		void read(void* dest, uint64_t bytes, uint64_t offset = 0) override;

	protected:
		MPQFile() = default;

		HANDLE mpq_file = nullptr;
		friend class MPQArchive;
	};

	class MPQArchive {
	public:
		void open(const QString& path);
		void close();

		MPQFile* openFile(const QString& path);
		void closeFile(MPQFile* file);

	protected:
		HANDLE mpq = nullptr;
	};

	class MPQFileSystem :public GameFileSystem {
	public:
		MPQFileSystem(const QString& root, const QString& locale);
		MPQFileSystem(MPQFileSystem&&) = default;
		virtual ~MPQFileSystem();

		constexpr QChar seperator() const override {
			return '\\';
		}

		std::future<void> load() override;

		ArchiveFile* openFile(const GameFileUri& uri) override;
		void closeFile(ArchiveFile* file) override;
		std::unique_ptr<std::list<GameFileUri::path_t>> fileList() override;

		GameFileUri asFileId(const GameFileUri& uri) override;
		GameFileUri asFilePath(const GameFileUri& uri) override;
		GameFileUri asInternal(const GameFileUri& uri) override;
		GameFileUri asInternal(const GameFileInfo& info) override;
		GameFileInfo asInfo(const GameFileUri& uri) override;

	protected:
		static const std::vector<QString> defaultMPQs;
		static const std::vector<QString> localeMPQs;	
		std::deque<std::pair<QString, std::unique_ptr<MPQArchive>>> mpqArchives;
	};
};