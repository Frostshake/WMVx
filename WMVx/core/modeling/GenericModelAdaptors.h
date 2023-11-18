#pragma once
#include "ModelAdaptors.h"
#include "VanillaM2Definitions.h"
#include "VanillaAnimation.h"
#include "WOTLKM2Definitions.h"
#include "WOTLKAnimation.h"
#include "BFAM2Definitions.h"
#include "BFAAnimation.h"
#include "../game/GameConstants.h"
#include "../database/GameDatasetAdaptors.h"
#include "Particles.h"

namespace core {

	template<typename T>
	class GenericModelGeosetAdaptor : public ModelGeosetAdaptor {
	public:
		GenericModelGeosetAdaptor(T* handle) {
			this->handle = handle;
		}
		GenericModelGeosetAdaptor(GenericModelGeosetAdaptor<T>&&) = default;

		virtual ~GenericModelGeosetAdaptor() {}

		constexpr virtual uint16_t getId() const {
			return handle->id;
		};
		constexpr virtual uint32_t getVertexStart() const {
			return handle->vertexStart;
		};
		constexpr virtual uint32_t getVertexCount() const {
			return handle->vertexCount;
		};
		constexpr virtual uint32_t getTriangleStart() const {
			return handle->triangleStart;
		}
		constexpr virtual uint32_t getTriangleCount() const {
			return handle->triangleCount;
		}

	protected:
		T* handle;
	};

	using VanillaModelGeosetAdaptor = GenericModelGeosetAdaptor<VanillaModelGeosetM2>;

	using WOTLKModelGeosetAdaptor = GenericModelGeosetAdaptor<WOTLKModelGeosetM2>;

	class BFAModelGeosetAdaptor : public GenericModelGeosetAdaptor<BFAModelGeosetM2> {
	public:
		BFAModelGeosetAdaptor(BFAModelGeosetM2* handle, uint32_t triangle_start_override) : GenericModelGeosetAdaptor(handle) {
			triangleStartOverride = triangle_start_override;
		}

		constexpr virtual uint32_t getTriangleStart() const {
			return this->triangleStartOverride;
		}
	protected:
		uint32_t triangleStartOverride;
	};

	template<typename T>
	class GenericModelAttachmentDefinitionAdaptor : public ModelAttachmentDefinitionAdaptor {
	public:
		GenericModelAttachmentDefinitionAdaptor(T* handle) {
			this->handle = handle;
		}
		GenericModelAttachmentDefinitionAdaptor(GenericModelAttachmentDefinitionAdaptor<T>&&) = default;

		virtual ~GenericModelAttachmentDefinitionAdaptor() {}

		constexpr virtual uint32_t getId() const {
			return handle->id;
		}
		constexpr virtual uint16_t getBone() const {
			return handle->bone;
		}
		virtual const Vector3& getPosition() const {
			return handle->position;
		}

	protected:
		T* handle;
	};

	using VanillaModelAttachmentDefinitionAdaptor = GenericModelAttachmentDefinitionAdaptor<VanillaModelAttachmentM2>;
	using WOTLKModelAttachmentDefinitionAdaptor = GenericModelAttachmentDefinitionAdaptor<WOTLKModelAttachmentM2>;
	using BFAModelAttachmentDefinitionAdaptor = GenericModelAttachmentDefinitionAdaptor<BFAModelAttachmentM2>;


	template<template<typename> class T>
	class GenericModelTextureAnimationAdaptor : public ModelTextureAnimationAdaptor {
	public:
		GenericModelTextureAnimationAdaptor() = default;
		GenericModelTextureAnimationAdaptor(GenericModelTextureAnimationAdaptor&&) = default;
		virtual ~GenericModelTextureAnimationAdaptor() {}

		//TODO protection?
		T<Vector3> translation;
		T<Vector3> rotation;	//TODO SHOULD THIS BE VECTOR4!? compare wowdev wiki to wmv old
		T<Vector3> scale;

	protected:

		virtual bool translationUses(size_t animation_index) const {
			return translation.uses(animation_index);
		}

		virtual const Vector3& translationValue(size_t animation_index, const AnimationTickArgs& tick) const {
			return translation.getValue(animation_index, tick);
		}

		virtual bool rotationUses(size_t animation_index) const {
			return rotation.uses(animation_index);
		}

		virtual const Vector3& rotationValue(size_t animation_index, const AnimationTickArgs& tick) const {
			return rotation.getValue(animation_index, tick);
		}

		virtual bool scaleUses(size_t animation_index) const {
			return scale.uses(animation_index);
		}

		virtual const Vector3& scaleValue(size_t animation_index, const AnimationTickArgs& tick) const {
			return scale.getValue(animation_index, tick);
		}
	};

	using VanillaModelTextureAnimationAdaptor = GenericModelTextureAnimationAdaptor<VanillaAnimated>;
	using WOTLKModelTextureAnimationAdaptor = GenericModelTextureAnimationAdaptor<WOTLKAnimated>;
	using BFAModelTextureAnimationAdaptor = GenericModelTextureAnimationAdaptor<BFAAnimated>;

	template<class T>
	class GenericModelParticleEmitterAdaptor : public ModelParticleEmitterAdaptor {
	public:
		GenericModelParticleEmitterAdaptor() = default;
		GenericModelParticleEmitterAdaptor(GenericModelParticleEmitterAdaptor&&) = default;
		virtual ~GenericModelParticleEmitterAdaptor() {}

		//TODO protect members

		T definition;

		Vector3 position;

		ParticleFactory::Generator generator;

		std::vector<TexCoordSet> tiles;
		std::list<Particle> particles;

		virtual const std::vector<TexCoordSet>& getTiles() const {
			return tiles;
		}

		virtual const std::list<Particle>& getParticles() const {
			return particles;
		}

		virtual const Vector3& getPosition() const {
			return position;
		}

		constexpr virtual uint16_t getBone() const {
			return definition.bone;
		}

		constexpr virtual uint32_t getFlags() const {
			return definition.flags;
		}

		constexpr virtual bool isBillboard() const {
			return !(definition.flags & 0x1000); //TODO dont hard code
		}

		virtual std::array<uint16_t, 2> getTextureDimension() const {
			//TODO check args are correct order
			return {
				definition.textureDimensionRows,
				definition.textureDimensionColumns
			};
		}

		constexpr virtual uint8_t getBlendType() const {
			return definition.blendType;
		}
	};

	/*
		legacy versions dont contain equivilant data either in the db or model, use hard coded constants instead.
		this is only needed for game versions without DB/CharComponentTextureSections
	*/
	class LegacyCharacterComponentTextureAdaptor : public CharacterComponentTextureAdaptor {
	public:
		constexpr uint32_t getLayoutId() const override {
			return 0;
		}

		constexpr int32_t getLayoutWidth() const override {
			return regionPxWidth;
		}

		constexpr int32_t getLayoutHeight() const override {
			return regionPxHeight;
		}

		std::map<CharacterRegion, CharacterRegionCoords> getRegions() const override {
			return 	{
				{ CharacterRegion::ARM_UPPER, {0, 0, 128 * REGION_FAC_X, 64 * REGION_FAC_Y} },	// arm upper
				{ CharacterRegion::ARM_LOWER, {0, 64 * REGION_FAC_Y, 128 * REGION_FAC_X, 64 * REGION_FAC_Y} },	// arm lower
				{ CharacterRegion::HAND, {0, 128 * REGION_FAC_Y, 128 * REGION_FAC_X, 32 * REGION_FAC_Y} },	// hand
				{ CharacterRegion::FACE_UPPER, {0, 160 * REGION_FAC_Y, 128 * REGION_FAC_X, 32 * REGION_FAC_Y} },	// face upper
				{ CharacterRegion::FACE_LOWER, {0, 192 * REGION_FAC_Y, 128 * REGION_FAC_X, 64 * REGION_FAC_Y} },	// face lower
				{ CharacterRegion::TORSO_UPPER, {128 * REGION_FAC_X, 0, 128 * REGION_FAC_X, 64 * REGION_FAC_Y} },	// torso upper
				{ CharacterRegion::TORSO_LOWER, {128 * REGION_FAC_X, 64 * REGION_FAC_Y, 128 * REGION_FAC_X, 32 * REGION_FAC_Y} },	// torso lower
				{ CharacterRegion::LEG_UPPER, {128 * REGION_FAC_X, 96 * REGION_FAC_Y, 128 * REGION_FAC_X, 64 * REGION_FAC_Y} }, // pelvis upper
				{ CharacterRegion::LEG_LOWER, {128 * REGION_FAC_X, 160 * REGION_FAC_Y, 128 * REGION_FAC_X, 64 * REGION_FAC_Y} },// pelvis lower
				{ CharacterRegion::FOOT, {128 * REGION_FAC_X, 224 * REGION_FAC_Y, 128 * REGION_FAC_X, 32 * REGION_FAC_Y} }	// foot
			};
		}


	protected:

		const int32_t REGION_FAC_X = 2;
		const int32_t REGION_FAC_Y = 2;

		const int32_t regionPxWidth = 256 * REGION_FAC_X;
		const int32_t regionPxHeight = 256 * REGION_FAC_Y;
	};

};