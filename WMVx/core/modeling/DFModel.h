#pragma once

#include "../filesystem/CascFileSystem.h"
#include "../database/DFGameDatabase.h"
#include "../utility/Color.h"
#include "../utility/Matrix.h"
#include "../modeling/RawModel.h"
#include "../modeling/BFAModel.h"

namespace core {

	class DFModel : public BFAModel /*RawModel*/
	{
	public:
		DFModel() = default;
		DFModel(DFModel&&) = default;
		virtual ~DFModel() = default;
	};

};
