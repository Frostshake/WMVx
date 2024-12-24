#pragma once

#include "../filesystem/CascFileSystem.h"
#include <vector>
#include <WDBReader/Database.hpp>

namespace core {

	template<typename BaseDataset, typename ImplAdaptor>
	class GenericDB2Dataset : public BaseDataset {
	public:
		using Adaptor = ImplAdaptor;
		GenericDB2Dataset(CascFileSystem* fs, const GameFileUri& uri) : BaseDataset()
		{
			auto file = fs->openFile(uri);
			if (file == nullptr) {
				throw std::runtime_error("Unable to open db2.");
			}

			auto db2 = WDBReader::Database::makeDB2File<ImplAdaptor::Record>(
				file->release()
			);

			for (auto& rec : *db2) {
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


	template<WDBReader::Database::TRecord T>
	class GenericDB2RecordAdaptor {
	public:
		using Record = T;
		GenericDB2RecordAdaptor(T&& record) : _record(std::move(record)) {}
		GenericDB2RecordAdaptor(GenericDB2RecordAdaptor<T>&&) = default;
		virtual ~GenericDB2RecordAdaptor() = default;
	protected:
		T _record;
	};

}