#pragma once

#include <vector>
#include "DBCBackedDataset.h"

namespace core {

	template
		<typename BaseDataset, typename ImplAdaptor, const char* filename>
		class GenericDBCDataset : public BaseDataset, public DBCBackedDataset<ImplAdaptor, typename BaseDataset::BaseAdaptor> {
		public:
			using Adaptor = ImplAdaptor;
			GenericDBCDataset(MPQFileSystem* fs) :
				BaseDataset(),
				DBCBackedDataset<ImplAdaptor, typename BaseDataset::BaseAdaptor>(fs, filename) {}

			GenericDBCDataset(GenericDBCDataset&&) = default;
			virtual ~GenericDBCDataset() {}

			const std::vector<typename BaseDataset::BaseAdaptor*>& all() const override {
				return reinterpret_cast<const std::vector<typename BaseDataset::BaseAdaptor*>&>(this->adaptors);
			}
	};
}