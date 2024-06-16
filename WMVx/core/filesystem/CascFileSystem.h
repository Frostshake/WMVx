#pragma once
#include <cstdint>
#include <memory>
#include <map>
#include "GameFileSystem.h"
#include <WDBReader/Filesystem/CASCFilesystem.hpp>

namespace core {
	class CascFile final : public ArchiveFile {
	public:
		CascFile(const GameFileUri& uri, std::unique_ptr<WDBReader::Filesystem::CASCFileSource> source) :
			_impl(std::move(source)), ArchiveFile(uri)
		{}
		uint64_t getFileSize() override;
		void read(void* dest, uint64_t bytes, uint64_t offset = 0) override;

	protected:
		std::unique_ptr<WDBReader::Filesystem::CASCFileSource> _impl;
	};

	class CascFileSystem final : public GameFileSystem
	{
	public:
		CascFileSystem(const QString& root, const QString& locale, const QString& list_file);
		CascFileSystem(CascFileSystem&&) = default;
		virtual ~CascFileSystem() = default;

		constexpr QChar seperator() const override {
			return '/';
		}

		std::future<void> load() override;

		std::unique_ptr<ArchiveFile> openFile(const GameFileUri& uri) override;
		std::unique_ptr<std::list<GameFileUri::path_t>> fileList() override;

		GameFileUri asFileId(const GameFileUri& uri) override;
		GameFileUri asFilePath(const GameFileUri& uri) override;
		GameFileUri asInternal(const GameFileUri& uri) override;
		GameFileUri asInternal(const GameFileInfo& info) override;
		GameFileInfo asInfo(const GameFileUri& uri) override;

	protected:
		void addExtraEncryptionKeys();
		inline void addFileMappingFromLine(const QString& line, qsizetype seperator_pos);

		std::unique_ptr<WDBReader::Filesystem::CASCFilesystem> _impl;
		std::unordered_map<GameFileUri::path_t, GameFileUri::id_t> fileNameToIdMap;
		std::unordered_map<GameFileUri::id_t, GameFileUri::path_t> idToFileNameMap;

		const QString listFilePath;
		int cascLocale;
	};

};