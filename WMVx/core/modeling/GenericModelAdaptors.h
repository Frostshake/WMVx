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

	template<M2_VER_RANGE R>
	class GenericModelGeosetAdaptor : public ModelGeosetAdaptor {
	public:
		GenericModelGeosetAdaptor(ModelGeosetM2<R>* handle) {
			this->handle = handle;
		}
		GenericModelGeosetAdaptor(GenericModelGeosetAdaptor<R>&&) = default;

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
		ModelGeosetM2<R>* handle;
	};


	template<M2_VER_RANGE R>
	class OverridableModelGeosetAdaptor : public GenericModelGeosetAdaptor<R> {
	public:
		OverridableModelGeosetAdaptor(ModelGeosetM2<R>* handle, uint32_t triangle_start_override) : GenericModelGeosetAdaptor<R>(handle) {
			triangleStartOverride = triangle_start_override;
		}

		constexpr virtual uint32_t getTriangleStart() const {
			return this->triangleStartOverride;
		}
	protected:
		uint32_t triangleStartOverride;
	};



	template<M2_VER_RANGE R>
	class GenericModelAttachmentDefinitionAdaptor : public ModelAttachmentDefinitionAdaptor {
	public:
		GenericModelAttachmentDefinitionAdaptor(ModelAttachmentM2<R>&& def) : 
			definition(std::move(def)) {}
		GenericModelAttachmentDefinitionAdaptor(GenericModelAttachmentDefinitionAdaptor<R>&&) = default;

		virtual ~GenericModelAttachmentDefinitionAdaptor() {}

		constexpr virtual uint32_t getId() const {
			return  definition.id;
		}
		constexpr virtual uint16_t getBone() const {
			return  definition.bone;
		}
		virtual const Vector3& getPosition() const {
			return definition.position;
		}

	protected:
		ModelAttachmentM2<R> definition;
	};



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


	template<M2_VER_RANGE R>
	class GenericModelAnimationSequenceAdaptor : public ModelAnimationSequenceAdaptor {
	public:
		GenericModelAnimationSequenceAdaptor(AnimationSequenceM2<R>&& def) : definition(std::move(def)) {}
		GenericModelAnimationSequenceAdaptor(GenericModelAnimationSequenceAdaptor&&) = default;

		virtual ~GenericModelAnimationSequenceAdaptor() {}

		constexpr virtual uint16_t getId() const {
			return definition.id;
		};

		constexpr virtual uint16_t getVariationId() const {
			return definition.variationId;
		};

		constexpr virtual uint32_t getDuration() const {
			constexpr auto has_timestamps = requires(AnimationSequenceM2<R> as) {
				{ as.endTimestamp }; 
				{ as.startTimestamp };
			};
			
			if constexpr (has_timestamps) {
				return definition.endTimestamp - definition.startTimestamp;
			}
			else {
				return definition.duration;
			}
		}

	protected:
		AnimationSequenceM2<R> definition;
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

	using ModelBoneAdaptorLegacy = GenericBoneAdaptor<LegacyAnimated<Vector3>, LegacyAnimated<Quaternion>, ModelBoneM2<M2_VER_RANGE(M2_VER_VANILLA_MIN, M2_VER_VANILLA_MAX)>>;
	template<M2_VER_RANGE R>
	using StandardModelBoneAdaptor = GenericBoneAdaptor<StandardAnimated<Vector3>, StandardAnimated<Quaternion, PACK_QUATERNION, Quat16ToQuat32>, ModelBoneM2<R>>;


	template<M2_VER_RANGE R>
	class GenericModelRibbonEmitter : public ModelRibbonEmitterAdaptor {
	public:

		virtual void update(size_t animation_index, const AnimationTickArgs& tick, std::vector<ModelBoneAdaptor*>& allbones) {

			const auto* parent_bone = allbones[definition.boneIndex];

			//TODO tidy code, better names, better logic

			Vector3 ntpos = parent_bone->getMat() * pos;
			Vector3 ntup = parent_bone->getMat() * (pos + Vector3(0, 0, 1));
			ntup -= ntpos;
			ntup.normalize();
			float dlen = (ntpos - tpos).length();

			// move first segment
			RibbonSegment& first = *segments.begin();
			if (first.len > definition.edgeLifetime) {
				// add new segment
				first.back = (tpos - ntpos).normalize();
				first.len0 = first.len;
				RibbonSegment newseg;
				newseg.position = ntpos;
				newseg.up = ntup;
				newseg.len = dlen;
				segments.push_front(newseg);
			}
			else {
				first.up = ntup;
				first.position = ntpos;
				first.len += dlen;
			}

			// kill stuff from the end
			float l = 0;
			bool erasemode = false;
			for (std::list<RibbonSegment>::iterator it = segments.begin(); it != segments.end(); ) {
				if (!erasemode) {
					l += it->len;
					if (l > length) {
						it->len = l - length;
						erasemode = true;
					}
					++it;
				}
				else {
					segments.erase(it++);
				}
			}

			tpos = ntpos;
			tcolor = Vector4(color.getValue(animation_index, tick), opacity.getValue(animation_index, tick));
			tabove = above.getValue(animation_index, tick);
			tbelow = below.getValue(animation_index, tick);
		}

		virtual const std::vector<uint16_t> getTexture() const {
			return textures;
		}

		virtual const Vector4& getTColor() const {
			return tcolor;
		}

		virtual float getLength() const {
			return length;
		}

		virtual float getTAbove() const {
			return tabove;
		}

		virtual float getTBelow() const {
			return tbelow;
		}

		virtual const std::list<ModelRibbonEmitterAdaptor::RibbonSegment>& getSegments() const {
			return segments;
		}

		ModelRibbonEmitterM2<R> definition;

		StandardAnimated<Vector3> color;
		StandardAnimated<float, int16_t, ShortToFloat> opacity;
		StandardAnimated<float> above;
		StandardAnimated<float> below;

		Vector3 pos;
		Vector3 tpos; //TODO better name
		Vector4 tcolor;
		int32_t numberOfSegments;
		float length;

		float tabove, tbelow;

		std::vector<uint16_t> textures;


		std::list<ModelRibbonEmitterAdaptor::RibbonSegment> segments;
	};
};