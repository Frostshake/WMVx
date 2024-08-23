#pragma once

#include "RawModel.h"
#include "VanillaM2Definitions.h"
#include "../utility/Exceptions.h"

namespace core {
	class VanillaModel : public RawModel
	{
	public:

		VanillaModel() = default;
		VanillaModel(VanillaModel&&) = default;
		virtual ~VanillaModel() = default;

		virtual void load(GameFileSystem* fs, GameFileUri uri, TextureCallback loadTexture);

		const VanillaModelHeaderM2& getHeader() const {
			return header;
		}

		const std::vector<AnimationSequenceM2Legacy>& getAnimationSequences() const {
			return animationSequences;
		}

		const std::vector<ModelGeosetM2Legacy>& getGeosets() const {
			return geosets;
		}


	protected:
		VanillaModelHeaderM2 header;

		std::vector<ModelGeosetM2Legacy> geosets;
		std::vector<AnimationSequenceM2Legacy> animationSequences;

		std::vector<ModelAttachmentM2Legacy> attachmentDefinitions;

	};
}