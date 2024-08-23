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

		const std::vector<AnimationSequenceM2<M2_VER_RANGE(M2_VER_VANILLA_MIN, M2_VER_VANILLA_MAX)>>& getAnimationSequences() const {
			return animationSequences;
		}

		const std::vector<ModelGeosetM2<M2_VER_RANGE(M2_VER_VANILLA_MIN, M2_VER_VANILLA_MAX)>>& getGeosets() const {
			return geosets;
		}


	protected:
		VanillaModelHeaderM2 header;

		std::vector<ModelGeosetM2<M2_VER_RANGE(M2_VER_VANILLA_MIN, M2_VER_VANILLA_MAX)>> geosets;
		std::vector<AnimationSequenceM2<M2_VER_RANGE(M2_VER_VANILLA_MIN, M2_VER_VANILLA_MAX)>> animationSequences;

		std::vector<ModelAttachmentM2<M2_VER_RANGE(M2_VER_VANILLA_MIN, M2_VER_VANILLA_MAX)>> attachmentDefinitions;

	};
}