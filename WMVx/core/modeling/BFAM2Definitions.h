#pragma once

#include <stdint.h>
#include "M2Defintions.h"
#include "../utility/Vector3.h"
#include "../utility/Vector2.h"
#include "WOTLKM2Definitions.h"
#include "M2ChunkDefinitions.h"
#include <type_traits>

namespace core {

	template<typename Base, size_t integer_bits, size_t decimal_bits> struct FixedPoint
	{
		Base decimal : decimal_bits;
		Base integer : integer_bits;
		Base sign : 1;
		static_assert(std::is_integral<Base>::value, "Integral required.");
		static_assert((sizeof(Base) * 8) == (decimal_bits + integer_bits + 1), "Fixed point size doesnt match base size.");

		float toFloat() const { 
			return (sign ? -1.0f : 1.0f) * (integer + decimal / float(1 << decimal_bits)); 
		}
	};

	using FixedPoint69 = FixedPoint<uint16_t, 6, 9>;
	using FixedPoint25 = FixedPoint<uint8_t, 2, 5>;

	struct Vector2FP69 {
		FixedPoint69 x;
		FixedPoint69 y;
	};
	
	using BFAAnimationSequenceM2 = WOTLKAnimationSequenceM2;
	using BFAAnimationBlockM2 = WOTLKAnimationBlockM2;
	using BFAModelBoneM2 = WOTLKModelBoneM2;
	using BFATextureAnimationM2 = WOTLKTextureAnimationM2;
	using BFAModelColorM2 = WOTLKModelColorM2;
	using BFAModelTransparencyM2 = WOTLKModelTransparencyM2;
	using BFAModelAttachmentM2 = WOTLKModelAttachmentM2;
	using BFAModelGeosetM2 = WOTLKModelGeosetM2;
	using BFAModelRibbonEmitterM2 = WOTLKModelRibbonEmitterM2;

	// view or skin.
	struct BFAModelViewM2 {
		uint8_t id[4];
		M2Array indices;
		M2Array triangles;
		M2Array properties;
		M2Array submeshes;
		M2Array textureUnits;
		uint32_t boneCountMax;
		M2Array shadowBatches;
	};

	using BFAModelParticleParamsM2 = WOTLKModelParticleParamsM2;

	struct BFAModelParticleEmitterM2 {

		uint32_t id;
		uint32_t flags;
		Vector3 position;
		uint16_t bone;
		struct                                  
		{
			uint16_t texture_0 : 5;
			uint16_t texture_1 : 5;
			uint16_t texture_2 : 5;
			uint16_t : 1;
		} texture;
		M2Array geometryModelFilename;
		M2Array recursionModelFilename;
		uint8_t blendType;
		uint8_t emitterType;
		uint16_t particleColorIndex;
		FixedPoint25 multiTextureParamX[2];
		uint16_t textureTileRotation;
		uint16_t textureDimensionColumns; //TODO wmv and wiki have columns and rows swapped!?
		uint16_t textureDimensionRows;
		BFAAnimationBlockM2 emissionSpeed;
		BFAAnimationBlockM2 speedVariation;
		BFAAnimationBlockM2 verticalRange;
		BFAAnimationBlockM2 horizontalRange;
		BFAAnimationBlockM2 gravity;
		BFAAnimationBlockM2 lifespan;
		float lifespanVary;
		BFAAnimationBlockM2 emissionRate;
		float emissionRateVary;
		BFAAnimationBlockM2 emissionAreaLength;
		BFAAnimationBlockM2 emissionAreaWidth;
		BFAAnimationBlockM2 zSource;
		BFAModelParticleParamsM2 params;
		BFAAnimationBlockM2 enabledIn;
		Vector2FP69 multiTextureParam0[2];
		Vector2FP69 multiTextureParam1[2];
	};

	struct BFAModelHeaderM2 {
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
		M2Array blendMapOverrides;	//TODO is this correct name?
	};

};