#pragma once

#include <cstdint>
#include <string>
#include "../utility/Vector3.h"
#include "../utility/Vector2.h"
#include "M2Defintions.h"

namespace core {


	struct WOTLKFakeAnimationBlockM2 {
		M2Array timestamps;
		M2Array keys;
	};

	struct WOTLKModelLightM2 {
		uint16_t type;
		int16_t bone;
		Vector3 position;
		AnimationBlockM2 ambientColor;
		AnimationBlockM2 ambientIntensity;
		AnimationBlockM2 diffuseColor;
		AnimationBlockM2 diffuseIntensity;
		AnimationBlockM2 attenuationStart;
		AnimationBlockM2 attenuationEnd;
		AnimationBlockM2 visiblity;
	};


	// view or skin.
	struct WOTLKModelViewM2 {
		uint8_t id[4];
		M2Array indices;
		M2Array triangles;
		M2Array properties;
		M2Array submeshes;
		M2Array textureUnits;
		uint32_t boneCountMax;
	};

	

	struct WOTLKModelRibbonEmitterM2 {
		uint32_t id;
		uint32_t boneIndex;
		Vector3 position;
		M2Array textures;
		M2Array materials;
		AnimationBlockM2 color;
		AnimationBlockM2 alpha;
		AnimationBlockM2 heightAbove;
		AnimationBlockM2 heightBelow;
		float edgesPerSecond;
		float edgeLifetime;
		float gravity;
		uint16_t textureRows;
		uint16_t textureCols;
		AnimationBlockM2 textureSlot;
		AnimationBlockM2 visibility;
		int16_t priorityPlane;
		int8_t ribbonColorIndex;
		int8_t textureTransformLookupIndex;
	};

	struct WOTLKModelParticleParamsM2 {
		WOTLKFakeAnimationBlockM2 color;
		WOTLKFakeAnimationBlockM2 opacity;
		WOTLKFakeAnimationBlockM2 scale;
		Vector2 scaleVary;
		WOTLKFakeAnimationBlockM2 headCellTrack;
		WOTLKFakeAnimationBlockM2 tailCellTrack;
		float tailLength;
		float twinkleSpeed;
		float twinklePercent;
		float twinkleScaleMin;
		float twinkleScaleMax;
		float burstMultiplier;
		float drag;
		float baseSpin;
		float baseSpinVary;
		float spin;
		float spinVary;
		Vector3 tumbleMin;
		Vector3 tumbleMax;
		Vector3 windVector;	//TODO wiki and wmv use different names?
		float windTime;
		float followSpeed1;
		float followScale1;
		float followSpeed2;
		float followScale2;
		M2Array splinePoints;
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
		AnimationBlockM2 emissionSpeed;
		AnimationBlockM2 speedVariation;
		AnimationBlockM2 verticalRange;
		AnimationBlockM2 horizontalRange;
		AnimationBlockM2 gravity;
		AnimationBlockM2 lifespan;
		float lifespanVary;
		AnimationBlockM2 emissionRate;
		float emissionRateVary;
		AnimationBlockM2 emissionAreaLength;
		AnimationBlockM2 emissionAreaWidth;
		AnimationBlockM2 zSource;
		WOTLKModelParticleParamsM2 params;
		AnimationBlockM2 enabledIn;
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