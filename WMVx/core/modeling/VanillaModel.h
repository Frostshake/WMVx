#pragma once

#include "RawModel.h"
#include "VanillaM2Definitions.h"
#include "VanillaModelSupport.h"
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

		const std::vector<VanillaAnimationSequenceM2>& getAnimationSequences() const {
			return animationSequences;
		}

		const std::vector<VanillaModelGeosetM2>& getGeosets() const {
			return geosets;
		}


	protected:
		VanillaModelHeaderM2 header;

		std::vector<VanillaModelGeosetM2> geosets;
		std::vector<VanillaAnimationSequenceM2> animationSequences;

		std::vector<VanillaModelAttachmentM2> attachmentDefinitions;

	};
}