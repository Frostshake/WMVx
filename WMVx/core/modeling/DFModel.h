#pragma once

#include "DFM2Definitions.h"
#include "../filesystem/CascFileSystem.h"
#include "../database/DFGameDatabase.h"
#include "../utility/Color.h"
#include "../utility/Matrix.h"
#include "../modeling/RawModel.h"
#include "../modeling/BFAModel.h"

namespace core {

	class DFModel : public RawModel
	{
	public:
		DFModel() = default;
		DFModel(DFModel&&) = default;
		virtual ~DFModel() {}

		virtual void load(GameFileSystem* fs, GameFileUri uri, TextureCallback loadTexture);



	protected:
		DFModelHeaderM2 header;

		//TODO DF

	};

};
