#pragma once

#include <cstdint>
#include <string>
#include "../utility/Vector3.h"
#include "../utility/Vector2.h"
#include "M2Defintions.h"

namespace core {




	struct WOTLKModelLightM2 {
		uint16_t type;
		int16_t bone;
		Vector3 position;
		AnimationBlockM2<M2_VER_RANGE::EXACT(M2_VER_WOTLK)> ambientColor;
		AnimationBlockM2<M2_VER_RANGE::EXACT(M2_VER_WOTLK)> ambientIntensity;
		AnimationBlockM2<M2_VER_RANGE::EXACT(M2_VER_WOTLK)> diffuseColor;
		AnimationBlockM2<M2_VER_RANGE::EXACT(M2_VER_WOTLK)> diffuseIntensity;
		AnimationBlockM2<M2_VER_RANGE::EXACT(M2_VER_WOTLK)> attenuationStart;
		AnimationBlockM2<M2_VER_RANGE::EXACT(M2_VER_WOTLK)> attenuationEnd;
		AnimationBlockM2<M2_VER_RANGE::EXACT(M2_VER_WOTLK)> visiblity;
	};





	struct WOTLKModelParticleEmitterM2 {
		uint32_t id;
		uint32_t flags;
		Vector3 position;
		uint16_t bone;
		uint16_t texture;
		M2Array geometryModelFilename;
		M2Array recursionModelFilename;
		uint8_t blendType;
		uint8_t emitterType;
		uint16_t particleColorIndex;
		uint8_t particleType;
		uint8_t headTail;
		uint16_t textureTileRotation;
		uint16_t textureDimensionColumns; //TODO wmv and wiki have columns and rows swapped!?
		uint16_t textureDimensionRows;
		AnimationBlockM2< M2_VER_RANGE::EXACT(M2_VER_WOTLK)> emissionSpeed;
		AnimationBlockM2<M2_VER_RANGE::EXACT(M2_VER_WOTLK)> speedVariation;
		AnimationBlockM2<M2_VER_RANGE::EXACT(M2_VER_WOTLK)> verticalRange;
		AnimationBlockM2<M2_VER_RANGE::EXACT(M2_VER_WOTLK)> horizontalRange;
		AnimationBlockM2<M2_VER_RANGE::EXACT(M2_VER_WOTLK)> gravity;
		AnimationBlockM2<M2_VER_RANGE::EXACT(M2_VER_WOTLK)> lifespan;
		float lifespanVary;
		AnimationBlockM2<M2_VER_RANGE::EXACT(M2_VER_WOTLK)> emissionRate;
		float emissionRateVary;
		AnimationBlockM2<M2_VER_RANGE::EXACT(M2_VER_WOTLK)> emissionAreaLength;
		AnimationBlockM2<M2_VER_RANGE::EXACT(M2_VER_WOTLK)> emissionAreaWidth;
		AnimationBlockM2<M2_VER_RANGE::EXACT(M2_VER_WOTLK)> zSource;
		ModelParticleParamsM2<M2_VER_RANGE::EXACT(M2_VER_WOTLK)> params;
		AnimationBlockM2<M2_VER_RANGE::EXACT(M2_VER_WOTLK)> enabledIn;
	};

	struct WOTLKModelHeaderM2 {
		uint8_t id[4];
		uint32_t version;
		M2Array name;
		uint32_t globalModelFlags;
		M2Array globalSequences;
		M2Array animations;
		M2Array animationLookup;
		M2Array bones;
		M2Array keyBoneLookup;
		M2Array vertices;
		uint32_t views;
		M2Array colors;
		M2Array textures;
		M2Array transparency;
		M2Array uvAnimations;
		M2Array textureReplace;
		M2Array renderFlags;
		M2Array boneLookup;
		M2Array textureLookup;
		M2Array textureUnits;
		M2Array transparencyLookup;
		M2Array uvAnimationLookup;

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
		M2Array cameras;
		M2Array cameraLookup;
		M2Array ribbonEmitters;
		M2Array particleEmitters;
		M2Array blendMapOverrides;

	};

}