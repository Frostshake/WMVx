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

	using VanillaModelGeosetAdaptor = GenericModelGeosetAdaptor<ModelGeosetM2Legacy>;
	using WOTLKModelGeosetAdaptor = GenericModelGeosetAdaptor<ModelGeosetM2>;

	class BFAModelGeosetAdaptor : public GenericModelGeosetAdaptor<ModelGeosetM2> {
	public:
		BFAModelGeosetAdaptor(ModelGeosetM2* handle, uint32_t triangle_start_override) : GenericModelGeosetAdaptor(handle) {
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

	using ModelAttachmentDefinitionAdaptorLegacy = GenericModelAttachmentDefinitionAdaptor<ModelAttachmentM2Legacy>;
	using StandardModelAttachmentDefinitionAdaptor = GenericModelAttachmentDefinitionAdaptor<ModelAttachmentM2>;


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

		virtual Vector3 translationValue(size_t animation_index, const AnimationTickArgs& tick) const {
			return translation.getValue(animation_index, tick);
		}

		virtual bool rotationUses(size_t animation_index) const {
			return rotation.uses(animation_index);
		}

		virtual Vector3 rotationValue(size_t animation_index, const AnimationTickArgs& tick) const {
			return rotation.getValue(animation_index, tick);
		}

		virtual bool scaleUses(size_t animation_index) const {
			return scale.uses(animation_index);
		}

		virtual Vector3 scaleValue(size_t animation_index, const AnimationTickArgs& tick) const {
			return scale.getValue(animation_index, tick);
		}
	};

	using ModelTextureAnimationAdaptorLegacy = GenericModelTextureAnimationAdaptor<LegacyAnimated>;
	using StandardModelTextureAnimationAdaptor = GenericModelTextureAnimationAdaptor<StandardAnimated>;


	template<template<typename...> typename T>
	class GenericModelColorAdaptor : public ModelColorAdaptor {
	public:
		GenericModelColorAdaptor() = default;
		GenericModelColorAdaptor(GenericModelColorAdaptor&&) = default;
		virtual ~GenericModelColorAdaptor() {}

		T<Vector3> color;
		T<float, int16_t, ShortToFloat> opacity;

		virtual bool colorUses(size_t animation_index) const {
			return color.uses(animation_index);
		}

		virtual Vector3 colorValue(size_t animation_index, const AnimationTickArgs& tick) const {
			return color.getValue(animation_index, tick);
		}

		virtual bool opacityUses(size_t animation_index) const {
			return opacity.uses(animation_index);
		}

		virtual float opacityValue(size_t animation_index, const AnimationTickArgs& tick) const {
			return opacity.getValue(animation_index, tick);
		}
	};

	using ModelColorAdaptorLegacy = GenericModelColorAdaptor<LegacyAnimated>;
	using StandardModelColorAdaptor = GenericModelColorAdaptor<StandardAnimated>;


	template<template<typename...> class T>
	class GenericModelTransparencyAdaptor : public ModelTransparencyAdaptor {
	public:
		GenericModelTransparencyAdaptor() = default;
		GenericModelTransparencyAdaptor(GenericModelTransparencyAdaptor&&) = default;
		virtual ~GenericModelTransparencyAdaptor() {}

		T<float, int16_t, ShortToFloat> transparency;

		virtual bool transparencyUses(size_t animation_index) const {
			return transparency.uses(animation_index);
		}

		virtual float transparencyValue(size_t animation_index, const AnimationTickArgs& tick) const {
			return transparency.getValue(animation_index, tick);
		}
	};

	using ModelTransparencyAdaptorLegacy = GenericModelTransparencyAdaptor<LegacyAnimated>;
	using StandardModelTransparencyAdaptor = GenericModelTransparencyAdaptor<StandardAnimated>;


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


	class StandardModelAnimationSequenceAdaptor : public ModelAnimationSequenceAdaptor {
	public:
		StandardModelAnimationSequenceAdaptor(AnimationSequenceM2* handle) {
			this->handle = handle;
		}
		StandardModelAnimationSequenceAdaptor(StandardModelAnimationSequenceAdaptor&&) = default;

		virtual ~StandardModelAnimationSequenceAdaptor() {}

		constexpr virtual uint16_t getId() const {
			return handle->id;
		};

		constexpr virtual uint16_t getVariationId() const {
			return handle->variationId;
		};

		constexpr virtual uint32_t getDuration() const {
			return handle->duration;
		}

	protected:
		AnimationSequenceM2* handle;
	};

	class ModelAnimationSequenceAdaptorLegacy : public ModelAnimationSequenceAdaptor {
	public:
		ModelAnimationSequenceAdaptorLegacy(AnimationSequenceM2Legacy* handle) {
			this->handle = handle;
		}
		ModelAnimationSequenceAdaptorLegacy(ModelAnimationSequenceAdaptorLegacy&&) = default;

		virtual ~ModelAnimationSequenceAdaptorLegacy() {}

		constexpr virtual uint16_t getId() const {
			return handle->id;
		};

		constexpr virtual uint16_t getVariationId() const {
			return handle->variationId;
		};

		constexpr virtual uint32_t getDuration() const {
			return handle->endTimestamp - handle->startTimestamp;
		}

	protected:
		AnimationSequenceM2Legacy* handle;
	};


	template<class V, class Q, class B>
	class GenericBoneAdaptor : public  ModelBoneAdaptor {
	public:
		GenericBoneAdaptor() = default;
		GenericBoneAdaptor(GenericBoneAdaptor&&) = default;
		virtual ~GenericBoneAdaptor() {}

		V translation;
		Q rotation;
		V scale;

		Vector3 pivot;
		Vector3 translationPivot;

		bool billboard;
		Matrix mat;
		Matrix mrot;

		B boneDefinition;

		bool calculated;

		virtual const AnimatedValue<Vector3>* getTranslation() const override {
			return &translation;
		}

		virtual const AnimatedValue<Quaternion>* getRotation() const override {
			return &rotation;
		}

		virtual const AnimatedValue<Vector3>* getScale() const override {
			return &scale;
		}

		virtual const Matrix& getMat() const {
			return mat;
		}

		virtual const Matrix& getMRot() const {
			return mrot;
		}

		virtual const Vector3& getTranslationPivot() const {
			return translationPivot;
		}

		virtual const Vector3& getPivot() const {
			return pivot;
		}

		virtual int16_t getParentBoneId() const {
			return boneDefinition.parentBoneId;
		}

		virtual void resetCalculated() {
			calculated = false;
		}

		virtual void calculateMatrix(size_t animation_index, const AnimationTickArgs& tick, std::vector<ModelBoneAdaptor*>& allbones) {

			if (calculated) {
				return;
			}

			Matrix m;
			Quaternion q;

			if (rotation.uses(animation_index) || scale.uses(animation_index) || translation.uses(animation_index) || billboard) {
				m.translation(pivot);

				if (translation.uses(animation_index)) {
					m *= Matrix::newTranslation(translation.getValue(animation_index, tick));
				}

				if (rotation.uses(animation_index)) {
					q = rotation.getValue(animation_index, tick);
					m *= Matrix::newQuatRotate(q);
				}

				if (scale.uses(animation_index)) {
					m *= Matrix::newScale(scale.getValue(animation_index, tick));
				}

				if (billboard) {
					//TODO
				}

				m *= Matrix::newTranslation(pivot * -1.0f);
			}
			else {
				m.unit();
			}

			if (boneDefinition.parentBoneId > -1) {
				allbones[boneDefinition.parentBoneId]->calculateMatrix(animation_index, tick, allbones);
				mat = allbones[boneDefinition.parentBoneId]->getMat() * m;
			}
			else {
				mat = m;
			}

			if (rotation.uses(animation_index)) {
				if (boneDefinition.parentBoneId >= 0) {
					mrot = allbones[boneDefinition.parentBoneId]->getMRot() * Matrix::newQuatRotate(q);
				}
				else {
					mrot = Matrix::newQuatRotate(q);
				}
			}
			else {
				mrot.unit();
			}

			//TODO
			translationPivot = mat * pivot;
			calculated = true;
		}
	};

	using ModelBoneAdaptorLegacy = GenericBoneAdaptor<LegacyAnimated<Vector3>, LegacyAnimated<Quaternion>, ModelBoneM2Legacy>;
	using StandardModelBoneAdaptor = GenericBoneAdaptor<StandardAnimated<Vector3>, StandardAnimated<Quaternion, PACK_QUATERNION, Quat16ToQuat32>, ModelBoneM2>;

};