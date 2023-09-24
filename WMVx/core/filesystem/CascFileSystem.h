#pragma once
#include <cstdint>
#include <memory>
#include <map>
#include "GameFileSystem.h"

namespace core {
	class CascFile : public ArchiveFile {
	public:

		uint64_t getFileSize() override;
		void read(void* dest, uint64_t bytes, uint64_t offset = 0) override;

	protected:
		CascFile() = default;

		HANDLE casc_file = nullptr;
		friend class CascFileSystem;
	};

	class CascFileSystem : public GameFileSystem
	{
	public:
		CascFileSystem(const QString& root);
		CascFileSystem(CascFileSystem&& instance);
		virtual ~CascFileSystem();

		constexpr QChar seperator() const override {
			return '/';
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
		void addExtraEncryptionKeys();
		inline void addFileMappingFromLine(const QString& line, qsizetype seperator_pos);

		HANDLE hStorage;
		std::map<GameFileUri::path_t, GameFileUri::id_t> fileNameToIdMap;
		std::map<GameFileUri::id_t, GameFileUri::path_t> idToFileNameMap;
	};

};