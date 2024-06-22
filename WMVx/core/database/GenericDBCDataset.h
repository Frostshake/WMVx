#pragma once

#include "../filesystem/MPQFileSystem.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <WDBReader/Database/DBCFile.hpp>

namespace core {

	template<typename BaseDataset, typename ImplAdaptor, WDBReader::Database::DBCVersion Version>
	class GenericLegacyDBCDataset : public BaseDataset {
	public:
		using Adaptor = ImplAdaptor;
		GenericLegacyDBCDataset(MPQFileSystem* fs, const GameFileUri& uri) :
			BaseDataset()
		{
			auto file = fs->openFile(uri);
			if (file == nullptr) {
				throw std::runtime_error("Unable to open dbc.");
			}

			auto dbc = WDBReader::Database::makeDBCFile<ImplAdaptor::Record, WDBReader::Filesystem::MPQFileSource>(Version);

			dbc.open(static_cast<MPQFile*>(file.get())->release());
			dbc.load();
			_adaptors.reserve(dbc.size());

			for (auto& rec : dbc) {
				_adaptors.push_back(
					std::make_unique<ImplAdaptor>(std::move(rec))
				);
			}
		}

		const std::vector<typename BaseDataset::BaseAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<typename BaseDataset::BaseAdaptor*>&>(this->_adaptors);
		}

	protected:
		std::vector<std::unique_ptr<ImplAdaptor>> _adaptors;
	};

	template<typename ImplAdaptor, typename ImplExtraAdaptor, WDBReader::Database::DBCVersion Version>
	class GenericLegacyDBCCreatureDisplayInfoDataset : public DatasetCreatureDisplay {
	public:
		using Adaptor = ImplAdaptor;
		GenericLegacyDBCCreatureDisplayInfoDataset(MPQFileSystem* fs, const GameFileUri& uri, const GameFileUri& extra_uri) :
			DatasetCreatureDisplay()
		{

			auto extra_file = fs->openFile(extra_uri);
			auto file = fs->openFile(uri);
			if (extra_file == nullptr || file == nullptr) {
				throw std::runtime_error("Unable to open dbc.");
			}

			auto extra_adaptors = loadExtras(std::move(extra_file));

			auto dbc = WDBReader::Database::makeDBCFile<ImplAdaptor::Record, WDBReader::Filesystem::MPQFileSource>(Version);
			dbc.open(static_cast<MPQFile*>(file.get())->release());
			dbc.load();

			_adaptors.reserve(dbc.size());

			for (auto& rec : dbc) {

				std::unique_ptr<ImplExtraAdaptor> extra = nullptr;
				if (rec.data.extendedDisplayInfoId > 0) {
					auto found = extra_adaptors.find(rec.data.extendedDisplayInfoId);
					if (found != extra_adaptors.end()) {
						extra = std::move(found->second);
					}
				}

				_adaptors.push_back(
					std::make_unique<ImplAdaptor>(std::move(rec), std::move(extra))
				);
			}
		}

		const std::vector<typename DatasetCreatureDisplay::BaseAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<typename DatasetCreatureDisplay::BaseAdaptor*>&>(this->_adaptors);
		}

	protected:

		std::unordered_map<uint32_t, std::unique_ptr<ImplExtraAdaptor>> loadExtras(std::unique_ptr<ArchiveFile> file) {
			auto dbc = WDBReader::Database::makeDBCFile<ImplExtraAdaptor::Record, WDBReader::Filesystem::MPQFileSource>(Version);
			dbc.open(static_cast<MPQFile*>(file.get())->release());
			dbc.load();

			std::unordered_map<uint32_t, std::unique_ptr<ImplExtraAdaptor>> result;
			result.reserve(dbc.size());

			for (auto & rec : dbc) {
				uint32_t id = rec.data.id;
				result[id] = std::make_unique<ImplExtraAdaptor>(std::move(rec));
			}

			return result;
		}

		std::vector<std::unique_ptr<ImplAdaptor>> _adaptors;
	};

	template<WDBReader::Database::TRecord T>
	class GenericLegacyDBCRecordAdaptor {
	public:
		using Record = T;
		GenericLegacyDBCRecordAdaptor(T&& record) : _record(std::move(record)) {}
		GenericLegacyDBCRecordAdaptor(GenericLegacyDBCRecordAdaptor<T>&&) = default;
		virtual ~GenericLegacyDBCRecordAdaptor() = default;
	protected:
		T _record;
	};


}