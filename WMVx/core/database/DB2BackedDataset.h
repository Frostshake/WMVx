#pragma once
#include "DB2File.h"
#include "../filesystem/CascFileSystem.h"
#include <QString>

namespace core {

	template<typename T>
	class DB2BackedAdaptor {
	public:
		using Record = T;
		using SectionView = DB2File<T>::SectionView;

		DB2BackedAdaptor(const T* handle, const DB2File<T>* db2, const SectionView* section_view)
			: handle(handle), db2(db2), section_view(section_view) {}
		DB2BackedAdaptor(DB2BackedAdaptor&&) = default;
		virtual ~DB2BackedAdaptor() {}

	protected:
		const T* handle;
		const DB2File<T>* db2;
		const SectionView* section_view;
	};


	template<typename Adaptor, typename BaseAdaptor, bool autoLoad = true>
	class DB2BackedDataset {
	public:
		DB2BackedDataset(CascFileSystem* fs, const QString& name)
		{
			db2 = std::make_unique <DB2File<typename Adaptor::Record>>(DB2File<typename Adaptor::Record>(name));
			db2->open(fs);

			if constexpr (autoLoad) {
				for (auto it = db2->cbegin(); it != db2->cend(); ++it) {
					adaptors.push_back(
						std::make_unique<Adaptor>(&(*it), db2.get(), &(it.section()))
					);
				}
			}
		}
		DB2BackedDataset(DB2BackedDataset<Adaptor, BaseAdaptor, autoLoad>&&) = default;
		virtual ~DB2BackedDataset() {}
	
	protected:
		std::unique_ptr<DB2File<typename Adaptor::Record>> db2;
		std::vector<std::unique_ptr<Adaptor>> adaptors;
	};
};