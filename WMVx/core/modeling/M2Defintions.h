#pragma once
#include <cstdint>
#include "../utility/Vector2.h"
#include "../utility/Vector3.h"

namespace core {

	constexpr uint16_t TEXTURE_MAX = 32;

	struct M2Array {
		uint32_t size;
		uint32_t offset;
	};

	struct M2Box {
		Vector3 min;
		Vector3 max;
	};

	struct ModelTextureM2 {
		uint32_t type;
		uint32_t flags;
		M2Array name;
	};
	struct ModelVertexM2 {

		constexpr static uint8_t BONE_COUNT = 4;

		Vector3 position;
		uint8_t boneWeights[BONE_COUNT];
		uint8_t bones[BONE_COUNT];
		Vector3 normal;
		Vector2 textureCoords;
		float unknown1;	//TODO check values, wiki has these 2 floats as a secondry textureCoords vector
		float unknown2;
	};

	struct ModelRenderFlagsM2 {
		uint16_t flags;
		uint16_t blend;
	};

	// texture unit or batch
	struct ModelTextureUnitM2 {
		uint16_t flags;	//TODO wow dev wiki has this as two int8's check why
		uint16_t sharderId;
		uint16_t submeshIndex;
		uint16_t submeshIndex2;
		int16_t colorIndex;
		uint16_t renderFlagsIndex;
		uint16_t textureUnitIndex;
		uint16_t mode;
		uint16_t textureId;
		uint16_t textureUnit2;
		uint16_t transparencyIndex;
		uint16_t textureAnimationId;
	};

	struct AnimationBlockHeader {
		uint32_t size;
		uint32_t offset;
	};

	struct CharacterRegionCoords {
		int32_t positionX;
		int32_t positionY;
		int32_t sizeX;
		int32_t sizeY;
	};
};