#pragma once

#include "BFAM2Definitions.h"
#include "../filesystem/CascFileSystem.h"
#include "../database/BFAGameDatabase.h"
#include "../utility/Color.h"
#include "../utility/Matrix.h"
#include "../modeling/RawModel.h"

namespace core {

	class BFAModel : public RawModel
	{
	public:
		BFAModel() = default;
		BFAModel(BFAModel&&) = default;
		virtual ~BFAModel() {}

		virtual void load(GameFileSystem* fs, GameFileUri uri, TextureCallback loadTexture);

		const std::vector<BFAAnimationSequenceM2>& getAnimationSequences() const {
			return animationSequences;
		}
		
		const std::vector<BFAModelGeosetM2>& getGeosets() const {
			return geosets;
		}

		const std::vector<BFAModelAttachmentM2>& getAttachmentDefinitions() const {
			return attachmentDefinitions;
		}


	protected:
		BFAModelHeaderM2 header;

		std::vector<BFAModelGeosetM2> geosets;
		std::vector<BFAAnimationSequenceM2> animationSequences;

		std::vector<BFAModelAttachmentM2> attachmentDefinitions;

	};

};
