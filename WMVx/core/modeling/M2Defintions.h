#pragma once
#include <optional>
#include <variant>
#include <cstdint>
#include "../utility/Vector2.h"
#include "../utility/Vector3.h"

namespace core {

	//TODO  move overloads into utility.
	template <class... Fs> struct Overload : Fs... { using Fs::operator()...; };
	template <class... Fs> Overload(Fs...) -> Overload<Fs...>;

	constexpr uint32_t M2_VER_MIN = 0;
	constexpr uint32_t M2_VER_MAX = UINT32_MAX;

	constexpr uint32_t M2_VER_ALPHA = 256;
	constexpr uint32_t M2_VER_VANILLA_MIN = 256;
	constexpr uint32_t M2_VER_VANILLA_MAX = 257;
	constexpr uint32_t M2_VER_TBC_MIN = 260;
	constexpr uint32_t M2_VER_TBC_MAX = 263;
	constexpr uint32_t M2_VER_WOTLK = 264;
	constexpr uint32_t M2_VER_CATA_MIN = 265;
	constexpr uint32_t M2_VER_CATA_MAX = 272;
	constexpr uint32_t M2_VER_MOP_WOD = 272;
	constexpr uint32_t M2_VER_LEGION_PLUS = 272;

	enum GlobalFlags : uint32_t {
		TILT_X = 0x1,
		TILT_Y = 0x2,
		UNK_0x4 = 0x4,
		// >= BC
		USE_BLEND_MAP_OVERRIDES = 0x8,
		UNK_0x10 = 0x10,
		// >+ Mists
		LOAD_PHYS_DATA = 0x20,
		UNK_0x40 = 0x40,
		// >= WOD
		UNK_0x80 = 0x80,
		CAMERA_RELATED = 0x100,
		// >= Legion
		NEW_PARTICLE_RCORD = 0x200,
		UNK_0x400 = 0x400,
		TEX_TRANSFORM_USE_BONE_SEQ = 0x800,
		UNK_0x1000 = 0x1000,
		CHUNKED_ANIM_0x2000 = 0x2000,
		UNK_0x4000 = 0x4000,
		UNK_0x8000 = 0x8000,
		UNK_0x10000 = 0x10000,
		UNK_0x20000 = 0x20000,
		UNK_0x40000 = 0x40000,
		UNK_0x80000 = 0x80000,
		UNK_0x100000 = 0x100000,
		UNK_0x200000 = 0x200000,
	};

	using M2Signature = std::array<uint8_t, 4>;

	static bool signatureCompare(M2Signature left, M2Signature right) {
		static_assert(sizeof(M2Signature) == sizeof(uint32_t));
		return *reinterpret_cast<uint32_t*>(left.data()) == *reinterpret_cast<uint32_t*>(right.data());
	}

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

	// > BC
	struct AnimationSequenceM2 {
		uint16_t id;
		uint16_t variationId;
		uint32_t duration;
		float movespeed;
		uint32_t flags;
		uint16_t frequency;
		uint16_t unused; //padding
		uint32_t minimumRepitions;
		uint32_t maximumRepitions;
		uint32_t blendTime;
		M2Box bounds;
		float boundsRadius;
		int16_t nextAnimationId;
		uint16_t aliasNextId;
	};

	struct AnimationBlockM2 {
		uint16_t interpolationType;
		int16_t globalSequence;
		M2Array timestamps;
		M2Array keys;
	};
	

	struct TextureAnimationM2 {
		AnimationBlockM2 translation;
		AnimationBlockM2 rotation;
		AnimationBlockM2 scale;
	};

	struct ModelColorM2 {
		AnimationBlockM2 color;
		AnimationBlockM2 opacity;
	};


	struct ModelTransparencyM2 {
		AnimationBlockM2 transparency;
	};

	struct ModelAttachmentM2 {
		uint32_t id;
		uint16_t bone;
		uint16_t unknown1;
		Vector3 position;
		AnimationBlockM2 unknown2;
	};

	struct ModelBoneM2 {
		int32_t keyBoneId;
		uint32_t flags;
		int16_t parentBoneId;
		uint16_t submeshId;
		uint32_t unknown1;
		AnimationBlockM2 translation;
		AnimationBlockM2 rotation;
		AnimationBlockM2 scale;
		Vector3 pivot;
	};

	// geoset or skin section.
	struct ModelGeosetM2 {
		uint16_t id;
		uint16_t level;
		uint16_t vertexStart;
		uint16_t vertexCount;
		uint16_t triangleStart;
		uint16_t triangleCount;
		uint16_t boneCount;
		uint16_t boneStart;
		uint16_t boneInfluences;
		uint16_t boneRoot;
		Vector3 centerMass;
		Vector3 centerBoundingBox;
		float radius;
	};

	/// Legacy
	// <= BC
	struct AnimationSequenceM2Legacy {
		uint16_t id;
		uint16_t variationId;
		uint32_t startTimestamp;
		uint32_t endTimestamp;
		float movespeed;
		uint32_t flags;
		uint16_t frequency;
		uint16_t unused; //padding
		uint32_t minimumRepitions;
		uint32_t maximumRepitions;
		uint32_t blendTime;
		M2Box bounds;
		float boundsRadius;
		int16_t nextAnimationId;
		uint16_t aliasNextId;
	};

	struct AnimationBlockM2Legacy {
		uint16_t interpolationType;
		int16_t globalSequence;
		M2Array ranges;
		M2Array timestamps;
		M2Array keys;
	};

	struct ModelColorM2Legacy {
		AnimationBlockM2Legacy color;
		AnimationBlockM2Legacy opacity;
	};

	struct ModelTransparencyM2Legacy {
		AnimationBlockM2Legacy transparency;
	};

	struct ModelBoneM2Legacy {
		int32_t keyBoneId;
		uint32_t flags;
		int16_t parentBoneId;
		uint16_t submeshId;
		AnimationBlockM2Legacy translation;
		AnimationBlockM2Legacy rotation;
		AnimationBlockM2Legacy scale;
		Vector3 pivot;
	};

	struct TextureAnimationM2Legacy {
		//TODO CHECK THIS is correct (untested)
		AnimationBlockM2Legacy translation;
		AnimationBlockM2Legacy rotation;
		AnimationBlockM2Legacy scale;
	};

	struct ModelAttachmentM2Legacy {
		uint32_t id;
		uint16_t bone;
		uint16_t unknown1;
		Vector3 position;
		AnimationBlockM2Legacy unknown2;
	};

	// view or skin.
	struct ModelViewM2Legacy {
		M2Array indices;
		M2Array triangles;
		M2Array properties;
		M2Array submeshes;
		M2Array textureUnits;
		uint32_t boneCountMax;
	};

	// geoset or skin section.
	struct ModelGeosetM2Legacy {
		uint16_t id;
		uint16_t level;
		uint16_t vertexStart;
		uint16_t vertexCount;
		uint16_t triangleStart;
		uint16_t triangleCount;
		uint16_t boneCount;
		uint16_t boneStart;
		uint16_t boneInfluences;
		uint16_t boneRoot;
		Vector3 centerMass;
	};

};