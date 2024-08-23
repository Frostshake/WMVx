#pragma once
#include "WOTLKM2Definitions.h"
#include "../filesystem/MPQFileSystem.h"
#include "../database/WOTLKGameDatabase.h"
#include "../utility/Color.h"
#include <set>
#include "../utility/Matrix.h"
#include "WOTLKAnimation.h"
#include "WOTLKModelSupport.h"
#include "RawModel.h"

namespace core {

	class WOTLKModel : public RawModel
	{
	public:
		WOTLKModel() = default;
		WOTLKModel(WOTLKModel&&) = default;
		virtual ~WOTLKModel() = default;

		virtual void load(GameFileSystem* fs, GameFileUri uri, TextureCallback loadTexture);

		const std::vector<AnimationSequenceM2>& getAnimationSequences() const {
			return animationSequences;
		}

		const std::vector<ModelGeosetM2>& getGeosets() const {
			return geosets;
		}

		const std::vector<ModelAttachmentM2>& getAttachmentDefinitions() const {
			return attachmentDefinitions;
		}

		const WOTLKModelHeaderM2& getHeader() const {
			return header;
		}

	protected:

		WOTLKModelHeaderM2 header;

		std::vector<ModelGeosetM2> geosets;
		std::vector<AnimationSequenceM2> animationSequences;


		std::vector<ModelAttachmentM2> attachmentDefinitions;
		

	};

};