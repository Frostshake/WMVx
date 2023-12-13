#pragma once
#include "DBCFile.h"
#include "../filesystem/MPQFileSystem.h"
#include <QString>

namespace core {
	template<typename T>
	class DBCBackedAdaptor {
	public:
		using Record = T;
		DBCBackedAdaptor(const T* handle, const DBCFile<T>* dbc) 
			: handle(handle), dbc(dbc) {}
		DBCBackedAdaptor(DBCBackedAdaptor<T>&&) = default;
		virtual ~DBCBackedAdaptor() {}

	protected:
		const T* handle;
		const DBCFile<T>* dbc;
	};

	template<typename Adaptor, typename BaseAdaptor, bool autoLoad = true>
	class DBCBackedDataset {
	public:
		DBCBackedDataset(MPQFileSystem* fs, const QString& name)
		{
			dbc = std::make_unique<DBCFile<typename Adaptor::Record>>(name);
			dbc->open(fs);

			if constexpr (autoLoad) {
				auto& records = dbc->getRecords();
				for (auto it = records.begin(); it != records.end(); ++it) {
					adaptors.push_back(
						std::make_unique<Adaptor>(&(*it), dbc.get())
					);
				}
			}
		}

		DBCBackedDataset(DBCBackedDataset<Adaptor, BaseAdaptor, autoLoad>&&) = default;
		virtual ~DBCBackedDataset() {}

	protected:
		std::unique_ptr<DBCFile<typename Adaptor::Record>> dbc;
		std::vector<std::unique_ptr<Adaptor>> adaptors;
	};
}