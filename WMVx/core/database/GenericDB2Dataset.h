#pragma once

#include <vector>
#include "DB2BackedDataset.h"

namespace core {

	template
		<typename BaseDataset, typename ImplAdaptor, const char* filename>
		class GenericDB2Dataset : public BaseDataset, public DB2BackedDataset<ImplAdaptor, typename BaseDataset::BaseAdaptor> {
		public:
			using Adaptor = ImplAdaptor;
			GenericDB2Dataset(CascFileSystem* fs) :
				BaseDataset(),
				DB2BackedDataset<ImplAdaptor, typename BaseDataset::BaseAdaptor>(fs, filename) {}
			GenericDB2Dataset(GenericDB2Dataset&&) = default;
			virtual ~GenericDB2Dataset() {}

			const std::vector<typename BaseDataset::BaseAdaptor*>& all() const override {
				return reinterpret_cast<const std::vector<typename BaseDataset::BaseAdaptor*>&>(this->adaptors);
			}
	};

}