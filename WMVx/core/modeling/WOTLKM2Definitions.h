#pragma once

#include <cstdint>
#include <string>
#include "../utility/Vector3.h"
#include "../utility/Vector2.h"
#include "M2Defintions.h"

namespace core {

	struct WOTLKAnimationSequenceM2 {
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

	struct WOTLKAnimationBlockM2 {
		uint16_t interpolationType;
		int16_t globalSequence;
		M2Array timestamps;
		M2Array keys;
	};

	struct WOTLKFakeAnimationBlockM2 {
		M2Array timestamps;
		M2Array keys;
	};

	struct WOTLKModelBoneM2 {
		int32_t keyBoneId;
		uint32_t flags;
		int16_t parentBoneId;
		uint16_t submeshId;
		uint32_t unknown1;
		WOTLKAnimationBlockM2 translation;
		WOTLKAnimationBlockM2 rotation;
		WOTLKAnimationBlockM2 scale;
		Vector3 pivot;
	};

	struct WOTLKTextureAnimationM2 {
		WOTLKAnimationBlockM2 translation;
		WOTLKAnimationBlockM2 rotation;
		WOTLKAnimationBlockM2 scale;
	};

	struct WOTLKModelColorM2 {
		WOTLKAnimationBlockM2 color;
		WOTLKAnimationBlockM2 opacity;
	};

	struct WOTLKModelLightM2 {
		uint16_t type;
		int16_t bone;
		Vector3 position;
		WOTLKAnimationBlockM2 ambientColor;
		WOTLKAnimationBlockM2 ambientIntensity;
		WOTLKAnimationBlockM2 diffuseColor;
		WOTLKAnimationBlockM2 diffuseIntensity;
		WOTLKAnimationBlockM2 attenuationStart;
		WOTLKAnimationBlockM2 attenuationEnd;
		WOTLKAnimationBlockM2 visiblity;
	};

	struct WOTLKModelTransparencyM2 {
		WOTLKAnimationBlockM2 transparency;
	};

	struct WOTLKModelAttachmentM2 {
		uint32_t id;
		uint16_t bone;
		uint16_t unknown1;
		Vector3 position;
		WOTLKAnimationBlockM2 unknown2;
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

	// geoset or skin section.
	struct WOTLKModelGeosetM2 {
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

	struct WOTLKModelRibbonEmitterM2 {
		uint32_t id;
		uint32_t boneIndex;
		Vector3 position;
		M2Array textures;
		M2Array materials;
		WOTLKAnimationBlockM2 color;
		WOTLKAnimationBlockM2 alpha;
		WOTLKAnimationBlockM2 heightAbove;
		WOTLKAnimationBlockM2 heightBelow;
		float edgesPerSecond;
		float edgeLifetime;
		float gravity;
		uint16_t textureRows;
		uint16_t textureCols;
		WOTLKAnimationBlockM2 textureSlot;
		WOTLKAnimationBlockM2 visibility;
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
		WOTLKAnimationBlockM2 emissionSpeed;
		WOTLKAnimationBlockM2 speedVariation;
		WOTLKAnimationBlockM2 verticalRange;
		WOTLKAnimationBlockM2 horizontalRange;
		WOTLKAnimationBlockM2 gravity;
		WOTLKAnimationBlockM2 lifespan;
		float lifespanVary;
		WOTLKAnimationBlockM2 emissionRate;
		float emissionRateVary;
		WOTLKAnimationBlockM2 emissionAreaLength;
		WOTLKAnimationBlockM2 emissionAreaWidth;
		WOTLKAnimationBlockM2 zSource;
		WOTLKModelParticleParamsM2 params;
		WOTLKAnimationBlockM2 enabledIn;
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