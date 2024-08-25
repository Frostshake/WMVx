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
	class GenericOldModelGeosetAdaptor : public ModelGeosetAdaptor {
	public:
		GenericOldModelGeosetAdaptor(ModelGeosetM2<R>* handle) {
			this->handle = handle;
		}
		GenericOldModelGeosetAdaptor(GenericOldModelGeosetAdaptor<R>&&) = default;

		virtual ~GenericOldModelGeosetAdaptor() {}

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

		constexpr virtual Vector3 getCenterMass() const {
			return handle->centerMass;
		}

	protected:
		ModelGeosetM2<R>* handle;
	};


	template<M2_VER_RANGE R>
	class OverridableOldModelGeosetAdaptor : public GenericOldModelGeosetAdaptor<R> {
	public:
		OverridableOldModelGeosetAdaptor(ModelGeosetM2<R>* handle, uint32_t triangle_start_override) : GenericOldModelGeosetAdaptor<R>(handle) {
			triangleStartOverride = triangle_start_override;
		}

		constexpr virtual uint32_t getTriangleStart() const {
			return this->triangleStartOverride;
		}
	protected:
		uint32_t triangleStartOverride;
	};





	template<M2_VER_RANGE R>
	class GenericModelGeosetAdaptor : public ModelGeosetAdaptor {
	public:
		GenericModelGeosetAdaptor(ModelGeosetM2<R>&& g) : def(std::move(g)) {}
		GenericModelGeosetAdaptor(GenericModelGeosetAdaptor<R>&&) = default;

		virtual ~GenericModelGeosetAdaptor() {}

		constexpr virtual uint16_t getId() const {
			return def.id;
		};
		constexpr virtual uint32_t getVertexStart() const {
			return def.vertexStart;
		};
		constexpr virtual uint32_t getVertexCount() const {
			return def.vertexCount;
		};
		constexpr virtual uint32_t getTriangleStart() const {
			return def.triangleStart;
		}
		constexpr virtual uint32_t getTriangleCount() const {
			return def.triangleCount;
		}

		constexpr virtual Vector3 getCenterMass() const {
			return def.centerMass;
		}

	protected:
		ModelGeosetM2<R> def;
	};


	template<M2_VER_RANGE R>
	class OverridableModelGeosetAdaptor : public GenericModelGeosetAdaptor<R> {
	public:
		OverridableModelGeosetAdaptor(ModelGeosetM2<R>&& g, uint32_t triangle_start_override) : GenericModelGeosetAdaptor<R>(std::move(g)) {
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
	class GenericModelOldTextureAnimationAdaptor : public ModelTextureAnimationAdaptor {
	public:
		GenericModelOldTextureAnimationAdaptor() = default;
		GenericModelOldTextureAnimationAdaptor(GenericModelOldTextureAnimationAdaptor&&) = default;
		virtual ~GenericModelOldTextureAnimationAdaptor() {}

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

	using ModelTextureAnimationAdaptorLegacy = GenericModelOldTextureAnimationAdaptor<LegacyAnimated>;
	using StandardModelTextureAnimationAdaptor = GenericModelOldTextureAnimationAdaptor<StandardAnimated>;

	template<M2_VER_RANGE R>
	class GenericModelTextureAnimationAdaptor : public ModelTextureAnimationAdaptor {
	public:
		GenericModelTextureAnimationAdaptor() = default;
		GenericModelTextureAnimationAdaptor(AnimatedValue<Vector3, R>&& t, AnimatedValue<Vector3, R>&& r, AnimatedValue < Vector3, R>&& s) :
			translation(std::move(t)),
			rotation(std::move(r)),
			scale(std::move(s))
		{}
		GenericModelTextureAnimationAdaptor(GenericModelTextureAnimationAdaptor&&) = default;
		virtual ~GenericModelTextureAnimationAdaptor() {}

		//TODO protection?
		AnimatedValue<Vector3,R> translation;
		AnimatedValue<Vector3, R> rotation;	//TODO SHOULD THIS BE VECTOR4!? compare wowdev wiki to wmv old
		AnimatedValue<Vector3, R> scale;

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


	template<template<typename...> typename T>
	class GenericOldModelColorAdaptor : public ModelColorAdaptor {
	public:
		GenericOldModelColorAdaptor() = default;
		GenericOldModelColorAdaptor(GenericOldModelColorAdaptor&&) = default;
		virtual ~GenericOldModelColorAdaptor() {}

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

	using ModelColorAdaptorLegacy = GenericOldModelColorAdaptor<LegacyAnimated>;
	using StandardModelColorAdaptor = GenericOldModelColorAdaptor<StandardAnimated>;

	template<M2_VER_RANGE R>
	class GenericModelColorAdaptor : public ModelColorAdaptor {
	public:
		GenericModelColorAdaptor() = default;
		GenericModelColorAdaptor(AnimatedValue<Vector3, R>&& c, AnimatedValue<float, R>&& o) :
			color(std::move(c)), opacity(std::move(o)) {}
		GenericModelColorAdaptor(GenericModelColorAdaptor&&) = default;
		virtual ~GenericModelColorAdaptor() {}

		AnimatedValue<Vector3, R> color;
		AnimatedValue<float, R> opacity;

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

	template<template<typename...> class T>
	class GenericOldModelTransparencyAdaptor : public ModelTransparencyAdaptor {
	public:
		GenericOldModelTransparencyAdaptor() = default;
		GenericOldModelTransparencyAdaptor(GenericOldModelTransparencyAdaptor&&) = default;
		virtual ~GenericOldModelTransparencyAdaptor() {}

		T<float, int16_t, ShortToFloat> transparency;

		virtual bool transparencyUses(size_t animation_index) const {
			return transparency.uses(animation_index);
		}

		virtual float transparencyValue(size_t animation_index, const AnimationTickArgs& tick) const {
			return transparency.getValue(animation_index, tick);
		}
	};

	using ModelTransparencyAdaptorLegacy = GenericOldModelTransparencyAdaptor<LegacyAnimated>;
	using StandardModelTransparencyAdaptor = GenericOldModelTransparencyAdaptor<StandardAnimated>;

	template<M2_VER_RANGE R>
	class GenericModelTransparencyAdaptor : public ModelTransparencyAdaptor {
	public:
		GenericModelTransparencyAdaptor() = default;
		GenericModelTransparencyAdaptor(AnimatedValue<float, R>&& t) :
			transparency(std::move(t)) {}
		GenericModelTransparencyAdaptor(GenericModelTransparencyAdaptor&&) = default;
		virtual ~GenericModelTransparencyAdaptor() {}

		AnimatedValue<float, R> transparency;

		virtual bool transparencyUses(size_t animation_index) const {
			return transparency.uses(animation_index);
		}

		virtual float transparencyValue(size_t animation_index, const AnimationTickArgs& tick) const {
			return transparency.getValue(animation_index, tick);
		}
	};


	template<class T>
	class GenericOldModelParticleEmitterAdaptor : public ModelParticleEmitterAdaptor {
	public:
		GenericOldModelParticleEmitterAdaptor() = default;
		GenericOldModelParticleEmitterAdaptor(GenericOldModelParticleEmitterAdaptor&&) = default;
		virtual ~GenericOldModelParticleEmitterAdaptor() {}

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
	//TODO move else where.
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
	class GenericOldBoneAdaptor : public  ModelBoneAdaptor {
	public:
		GenericOldBoneAdaptor() = default;
		GenericOldBoneAdaptor(GenericOldBoneAdaptor&&) = default;
		virtual ~GenericOldBoneAdaptor() {}

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

		virtual const IAnimatedValue<Vector3>* getTranslation() const override {
			return &translation;
		}

		virtual const IAnimatedValue<Quaternion>* getRotation() const override {
			return &rotation;
		}

		virtual const IAnimatedValue<Vector3>* getScale() const override {
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

	using ModelBoneAdaptorLegacy = GenericOldBoneAdaptor<LegacyAnimated<Vector3>, LegacyAnimated<Quaternion>, ModelBoneM2<M2_VER_RANGE(M2_VER_VANILLA_MIN, M2_VER_VANILLA_MAX)>>;
	template<M2_VER_RANGE R>
	using StandardModelBoneAdaptor = GenericOldBoneAdaptor<StandardAnimated<Vector3>, StandardAnimated<Quaternion, PACK_QUATERNION, Quat16ToQuat32>, ModelBoneM2<R>>;


	template<M2_VER_RANGE R>
	class GenericModelBoneAdaptor : public  ModelBoneAdaptor {
	public:
		GenericModelBoneAdaptor() = default;
		GenericModelBoneAdaptor(ModelBoneM2<R>&& def, AnimatedValue<Vector3, R>&& t, AnimatedValue<Quaternion, R>&& r, AnimatedValue < Vector3, R>&& s) :
			boneDefinition(std::move(def)),
			translation(std::move(t)),
			rotation(std::move(r)),
			scale(std::move(s)),
			calculated(false)
		{
			pivot = Vector3::yUpToZUp(boneDefinition.pivot);
			billboard = (boneDefinition.flags & ModelBoneFlags::spherical_billboard) != 0;
		}
		GenericModelBoneAdaptor(GenericModelBoneAdaptor&&) = default;
		virtual ~GenericModelBoneAdaptor() {}

		AnimatedValue<Vector3, R> translation;
		AnimatedValue<Quaternion, R> rotation;
		AnimatedValue<Vector3, R> scale;

		Vector3 pivot;
		Vector3 translationPivot;

		bool billboard;
		Matrix mat;
		Matrix mrot;

		ModelBoneM2<R> boneDefinition;

		bool calculated;

		virtual const IAnimatedValue<Vector3>* getTranslation() const override {
			return &translation;
		}

		virtual const IAnimatedValue<Quaternion>* getRotation() const override {
			return &rotation;
		}

		virtual const IAnimatedValue<Vector3>* getScale() const override {
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




	template<M2_VER_RANGE R>
	class GenericOldModelRibbonEmitter : public ModelRibbonEmitterAdaptor {
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

	template<M2_VER_RANGE R>
	class GenericModelRibbonEmitter : public ModelRibbonEmitterAdaptor {
	public:
		GenericModelRibbonEmitter() = default;
		GenericModelRibbonEmitter(ModelRibbonEmitterM2<R>&& def, AnimatedValue<Vector3, R>&& c, AnimatedValue<float, R>&& o,
			AnimatedValue<float, R>&& a, AnimatedValue<float, R>&& b) :
			definition(std::move(def)),
			color(std::move(c)),
			opacity(std::move(o)),
			above(std::move(a)),
			below(std::move(b))
		{
			pos = Vector3::yUpToZUp(definition.position);
			tpos = Vector3::yUpToZUp(definition.position);

			numberOfSegments = (uint32_t)definition.edgesPerSecond;
			length = definition.edgesPerSecond * definition.edgeLifetime;

			auto segment = ModelRibbonEmitterAdaptor::RibbonSegment();
			segment.position = tpos;
			segment.len = 0;
			segments.push_back(std::move(segment));

		}
		GenericModelRibbonEmitter(GenericModelRibbonEmitter&&) = default;
		virtual ~GenericModelRibbonEmitter() {}

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

		AnimatedValue<Vector3, R> color;
		AnimatedValue<float, R> opacity;
		AnimatedValue<float, R> above;
		AnimatedValue<float, R> below;

		Vector3 pos;
		Vector3 tpos; //TODO better name
		Vector4 tcolor;
		int32_t numberOfSegments;
		float length;

		float tabove, tbelow;

		std::vector<uint16_t> textures;


		std::list<ModelRibbonEmitterAdaptor::RibbonSegment> segments;
	};



	template<M2_VER_RANGE R>
	class GenericModelParticleEmitterAdaptor : public ModelParticleEmitterAdaptor {
	public:
		GenericModelParticleEmitterAdaptor() = default;
		GenericModelParticleEmitterAdaptor(GenericModelParticleEmitterAdaptor&&) = default;
		virtual ~GenericModelParticleEmitterAdaptor() {}

		//TODO protect members

		ModelParticleEmitterM2<R> definition;

		Vector3 position;

		ParticleFactory::Generator generator;

		std::vector<TexCoordSet> tiles;
		std::list<Particle> particles;


		//TODO better names
		AnimatedValue<float, R> speed;
		AnimatedValue<float, R> variation;
		AnimatedValue<float, R> spread;
		AnimatedValue<float, R> lat;
		AnimatedValue<float, R> gravity;
		AnimatedValue<float, R> lifespan;
		AnimatedValue<float, R> rate;
		AnimatedValue<float, R> areal;
		AnimatedValue<float, R> areaw;
		AnimatedValue<float, R> deacceleration;
		AnimatedValue<float, R> enabled;

		Vector4 colors[3];	//TODO why fixed size?
		float sizes[3];

		float rem;

		virtual const std::vector<TexCoordSet>& getTiles() const override {
			return tiles;
		}

		virtual const std::list<Particle>& getParticles() const override {
			return particles;
		}

		virtual const Vector3& getPosition() const override {
			return position;
		}

		constexpr virtual uint16_t getBone() const override {
			return definition.bone;
		}

		constexpr virtual uint32_t getFlags() const override {
			return definition.flags;
		}

		constexpr virtual bool isBillboard() const override {
			return !(definition.flags & 0x1000); //TODO dont hard code
		}

		virtual std::array<uint16_t, 2> getTextureDimension() const override {
			//TODO check args are correct order
			return {
				definition.textureDimensionRows,
				definition.textureDimensionColumns
			};
		}

		constexpr virtual uint8_t getBlendType() const override {
			return definition.blendType;
		}

		constexpr virtual uint8_t getParticleType() const override {
			constexpr auto has_particle_type = requires(ModelParticleEmitterM2<R> r) {
				{ r.particleType };
			};

			if constexpr (has_particle_type) {
				return definition.particleType;
			}
			else {
				return 0;
			}
		}

		virtual float lifespanValue(size_t animation_index, const AnimationTickArgs& tick) const override {
			return lifespan.getValue(animation_index, tick);
		}

		//TODO getTexture should return std::array.

		virtual const std::vector<uint16_t> getTexture() const override {

			if constexpr (M2_VER_CONDITION_AFTER<M2_VER_CATA_MIN>::eval(R)) {

				if (definition.flags & ModelParticleFlags::MULTITEXTURE) {
					return {
						definition.texture.texture_0,
						definition.texture.texture_1,
						definition.texture.texture_2,
					};
				}

				return { definition.texture.texture_0 };
			}
			else {
				return { definition.texture };
			}
		}

		virtual void update(size_t animation_index, const AnimationTickArgs& tick, std::vector<ModelBoneAdaptor*>& allbones) override {
			const float deltat = tick.deltaTime / 1000.0f;

			size_t l_manim = animation_index;
			/*if (bZeroParticle)	//TODO what is zero particle?
			* l_manim = 0;
				;*/

			l_manim = 0;	//TODO DEBUG REMOVE - TESTING ONLY - SHOULD USE CONDITION ABOVE

			float grav = gravity.getValue(animation_index, tick);
			float deaccel = deacceleration.getValue(animation_index, tick);

			//TODO not sure why gravity is sometimes NaN...
			if (isnan(grav)) {
				grav = 0;
			}

			//TODO is condition needed/valid?
			if (generator) {
				float frate = rate.getValue(l_manim, tick);
				float flife = lifespan.getValue(l_manim, tick);
				float ftospawn;
				if (flife)
					ftospawn = (deltat * frate / flife) + rem;
				else
					ftospawn = rem;

				if (ftospawn < 1.0f) {
					rem = ftospawn;
					if (rem < 0)
						rem = 0;
				}
				else {
					unsigned int tospawn = (int)ftospawn;

					if ((tospawn + particles.size()) > MAX_PARTICLES) // Error check to prevent the program from trying to load insane amounts of particles.
						tospawn = (unsigned int)(MAX_PARTICLES - particles.size());

					rem = ftospawn - (float)tospawn;

					float w = areal.getValue(l_manim, tick) * 0.5f;
					float l = areaw.getValue(l_manim, tick) * 0.5f;
					float spd = speed.getValue(l_manim, tick);
					float var = variation.getValue(l_manim, tick);
					float spr = spread.getValue(l_manim, tick);
					float spr2 = lat.getValue(l_manim, tick);
					bool en = true;
					if (enabled.uses(animation_index))
						en = enabled.getValue(animation_index, tick) != 0;

					//rem = 0;
					if (en) {
						for (size_t i = 0; i < tospawn; i++) {
							Particle p = generator(
								this,
								animation_index,
								tick,
								allbones,
								{ w, l, spd, var, spr, spr2 }
							);
							// sanity check:
							if (particles.size() < MAX_PARTICLES) // No need to check this every loop iteration. Already checked above.
								particles.push_back(std::move(p));
						}
					}
				}

			}

			float mspeed = 1.0f;

			float mid = 0.5f; //TODO WHERE SHOULD THIS LIVE?

			//TODO tidy loop
			for (auto it = particles.begin(); it != particles.end(); ) {
				auto& particle = *it;
				particle.speed += particle.down * grav * deltat - particle.dir * deaccel * deltat;

				if (definition.drag > 0) {
					mspeed = expf(-1.0f * definition.drag * particle.life);
				}

				particle.position += particle.speed * mspeed * deltat;
				assert(!isnan(particle.position.x));

				particle.life += deltat;
				float rlife = particle.life / particle.maxlife;

				particle.size = lifeRamp<float>(rlife, mid, sizes[0], sizes[1], sizes[2]);
				particle.color = lifeRamp<Vector4>(rlife, mid, colors[0], colors[1], colors[2]);

				if (rlife >= 1.0f) {
					particles.erase(it++);
				}
				else {
					++it;
				}
			}

		}
	};
};