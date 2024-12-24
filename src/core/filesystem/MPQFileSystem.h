#pragma once

#include <deque>
#include <vector>
#include "GameFileSystem.h"
#include <WDBReader/Filesystem/MPQFilesystem.hpp>

namespace core {

	class MPQFile final : public ArchiveFile {
	public:
		MPQFile(const GameFileUri& uri, std::unique_ptr<WDBReader::Filesystem::MPQFileSource> source) :
			_impl(std::move(source)), ArchiveFile(uri)
		{}

		uint64_t getFileSize() override;
		void read(void* dest, uint64_t bytes, uint64_t offset = 0) override;
		std::unique_ptr<WDBReader::Filesystem::FileSource> release() override {
			return std::move(_impl);
		}

	protected:
		std::unique_ptr<WDBReader::Filesystem::MPQFileSource> _impl;
	};

	class MPQFileSystem final : public GameFileSystem {
	public:
		MPQFileSystem(const QString& root, const QString& locale);
		MPQFileSystem(MPQFileSystem&&) = default;
		virtual ~MPQFileSystem() = default;

		constexpr QChar seperator() const override {
			return '\\';
		}

		std::future<void> load() override;

		std::unique_ptr<ArchiveFile> openFile(const GameFileUri& uri) override;
		std::unique_ptr<std::vector<GameFileUri::path_t>> fileList(std::function<bool(const GameFileUri::path_t&)> pred) override;

		GameFileUri asFileId(const GameFileUri& uri) override;
		GameFileUri asFilePath(const GameFileUri& uri) override;
		GameFileUri asInternal(const GameFileUri& uri) override;
		GameFileUri asInternal(const GameFileInfo& info) override;
		GameFileInfo asInfo(const GameFileUri& uri) override;

	protected:
		std::unique_ptr<WDBReader::Filesystem::MPQFilesystem> _impl;
	};
};