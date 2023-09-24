#pragma once

#include <cstdint>
#include "M2Defintions.h"
#include "../utility/Vector2.h"
#include "../utility/Vector3.h"

//TODO vanilla animation structures are untested!

namespace core {

	struct VanillaAnimationSequenceM2 {
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

	struct VanillaAnimationBlockM2 {
		uint16_t type;
		int16_t sequence;
		M2Array ranges;
		M2Array timestamps;
		M2Array keys;
	};

	struct VanillaModelBoneM2 {
		int32_t keyBoneId;
		uint32_t flags;
		int16_t parentBoneId;
		uint16_t submeshId;
		VanillaAnimationBlockM2 translation;
		VanillaAnimationBlockM2 rotation;
		VanillaAnimationBlockM2 scale;
		Vector3 pivot;
	};

	struct VanillaTextureAnimationM2 {
		//TODO CHECK THIS is correct (untested)
		VanillaAnimationBlockM2 translation;
		VanillaAnimationBlockM2 rotation;
		VanillaAnimationBlockM2 scale;
	};


	struct VanillaModelColorM2 {
		VanillaAnimationBlockM2 color;
		VanillaAnimationBlockM2 opacity;
	};

	struct VanillaModelTransparencyM2 {
		VanillaAnimationBlockM2 transparency;
	};

	struct VanillaModelAttachmentM2 {
		uint32_t id;
		uint16_t bone;
		uint16_t unknown1;
		Vector3 position;
		VanillaAnimationBlockM2 unknown2;
	};

	// view or skin.
	struct VanillaModelViewM2 {
		M2Array indices;
		M2Array triangles;
		M2Array properties;
		M2Array submeshes;
		M2Array textureUnits;
		uint32_t boneCountMax;
	};

	// geoset or skin section.
	struct VanillaModelGeosetM2 {
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


	struct VanillaModelHeaderM2 {
		uint8_t id[4];
		uint32_t version;
		M2Array name;
		uint32_t globalModelFlags;
		M2Array globalSequences;
		M2Array animations;
		M2Array animationLookup;
		M2Array playableAnimationLookup;
		M2Array bones;
		M2Array keyBoneLookup;
		M2Array vertices;
		M2Array views;
		M2Array colors;
		M2Array textures;
		M2Array transparency;
		M2Array textureFlipbooks;
		M2Array uvAnimations;
		M2Array textureReplace;
		M2Array renderFlags;
		M2Array boneLookup;
		M2Array textureLookup;
		M2Array textureUnits;
		M2Array transparencyLookup;
		M2Array uvAnimationLookup;

		//TODO wow-wiki vs wmv has these field different, work out why.
		M2Box boundingBox;
		float boundingSphereRadius;
		M2Box collisionBox;
		float collisionSphereRadius;

		M2Array boundingTriangles;
		M2Array boundingVertices;
		M2Array boundingNormals;
		M2Array attachments;
		M2Array attachmentLookup;
		M2Array events;
		M2Array lights;
		M2Array cameraLookup;
		M2Array ribbonEmitters;
		M2Array particleEmitters;
	};
};