#pragma once

#include <cstdint>
#include "M2Defintions.h"
#include "../utility/Vector2.h"
#include "../utility/Vector3.h"

//TODO vanilla animation structures are untested!

namespace core {


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