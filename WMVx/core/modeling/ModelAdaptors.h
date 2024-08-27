#pragma once
#include <cstdint>
#include <vector>
#include "../utility/Vector3.h"
#include "../utility/Vector2.h"
#include "../utility/Matrix.h"
#include "Animation.h"
#include "Texture.h"

namespace core {
	class ModelGeosetAdaptor {
	public:

		ModelGeosetAdaptor() = default;
		ModelGeosetAdaptor(ModelGeosetAdaptor&&) = default;
		virtual ~ModelGeosetAdaptor() {}

		constexpr virtual uint16_t getId() const = 0;
		constexpr virtual uint32_t getVertexStart() const = 0;
		constexpr virtual uint32_t getVertexCount() const = 0;
		constexpr virtual uint32_t getTriangleStart() const = 0;
		constexpr virtual uint32_t getTriangleCount() const = 0;
		constexpr virtual Vector3 getCenterMass() const = 0;
	};

	class ModelAttachmentDefinitionAdaptor {
	public:
		ModelAttachmentDefinitionAdaptor() = default;
		ModelAttachmentDefinitionAdaptor(ModelAttachmentDefinitionAdaptor&&) = default;
		virtual ~ModelAttachmentDefinitionAdaptor() {}

		constexpr virtual uint32_t getId() const = 0;
		constexpr virtual uint16_t getBone() const = 0;
		virtual const Vector3& getPosition() const = 0;
	};

	class ModelAnimationSequenceAdaptor {
	public:
		ModelAnimationSequenceAdaptor() = default;
		ModelAnimationSequenceAdaptor(ModelAnimationSequenceAdaptor&&) = default;
		virtual ~ModelAnimationSequenceAdaptor() {}

		constexpr virtual uint16_t getId() const = 0;
		constexpr virtual uint16_t getVariationId() const = 0;
		constexpr virtual uint32_t getDuration() const = 0;
	};

	class ModelTextureAnimationAdaptor {
	public:
		ModelTextureAnimationAdaptor() = default;
		ModelTextureAnimationAdaptor(ModelTextureAnimationAdaptor&&) = default;
		virtual ~ModelTextureAnimationAdaptor() {}

		virtual bool translationUses(size_t animation_index) const = 0;

		virtual Vector3 translationValue(size_t animation_index, const AnimationTickArgs& tick) const = 0;

		virtual bool rotationUses(size_t animation_index) const = 0;

		virtual Vector3 rotationValue(size_t animation_index, const AnimationTickArgs& tick) const = 0;

		virtual bool scaleUses(size_t animation_index) const = 0;

		virtual Vector3 scaleValue(size_t animation_index, const AnimationTickArgs& tick) const = 0;
	};

	class ModelColorAdaptor {
	public:
		ModelColorAdaptor() = default;
		ModelColorAdaptor(ModelColorAdaptor&&) = default;
		virtual ~ModelColorAdaptor() {}

		virtual bool colorUses(size_t animation_index) const = 0;
		virtual Vector3 colorValue(size_t animation_index, const AnimationTickArgs& tick) const = 0;

		virtual bool opacityUses(size_t animation_index) const = 0;
		virtual float opacityValue(size_t animation_index, const AnimationTickArgs& tick) const = 0;
	};

	class ModelTransparencyAdaptor {
	public:
		ModelTransparencyAdaptor() = default;
		ModelTransparencyAdaptor(ModelTransparencyAdaptor&&) = default;
		virtual ~ModelTransparencyAdaptor() {}

		virtual bool transparencyUses(size_t animation_index) const = 0;
		virtual float transparencyValue(size_t animation_index, const AnimationTickArgs& tick) const = 0;
	};

	class ModelBoneAdaptor {
	public:
		ModelBoneAdaptor() = default;
		ModelBoneAdaptor(ModelBoneAdaptor&&) = default;
		virtual ~ModelBoneAdaptor() {}

		virtual const IAnimatedValue<Vector3>* getTranslation() const = 0;
		virtual const IAnimatedValue<Quaternion>* getRotation() const = 0;
		virtual const IAnimatedValue<Vector3>* getScale() const = 0;

		virtual void calculateMatrix(size_t animation_index, const AnimationTickArgs& tick, std::vector<ModelBoneAdaptor*>& allbones) = 0;

		virtual const Matrix& getMat() const = 0;
		virtual const Matrix& getMRot() const = 0;

		virtual const Vector3& getTranslationPivot() const = 0;
		virtual const Vector3& getPivot() const = 0;

		virtual int16_t getParentBoneId() const = 0;

		virtual void resetCalculated() = 0;
	};


	class ModelRibbonEmitterAdaptor {
	public:

		struct RibbonSegment {
			Vector3 position;
			Vector3 up;
			Vector3 back;
			float len, len0;
		};

		ModelRibbonEmitterAdaptor() = default;
		ModelRibbonEmitterAdaptor(ModelRibbonEmitterAdaptor&&) = default;
		virtual ~ModelRibbonEmitterAdaptor() {}

		virtual void update(size_t animation_index, const AnimationTickArgs& tick, std::vector<ModelBoneAdaptor*>& allbones) = 0;

		virtual const std::vector<uint16_t> getTexture() const = 0;

		virtual const Vector4& getTColor() const = 0;

		virtual float getLength() const = 0;

		virtual float getTAbove() const = 0;

		virtual float getTBelow() const = 0;

		virtual const std::list<RibbonSegment>& getSegments() const = 0;
	};

	class ModelParticleEmitterAdaptor {
	public:
		const size_t MAX_PARTICLES = 10000;

		struct Particle {
			Vector3 position;
			Vector3 speed, down, origin, dir;
			Vector3	corners[4];
			Vector3 tpos;	//TODO tpos is added in BFA WMV, check code, this isnt used in WMVX codebase yet.
			float size, life, maxlife;
			size_t tile;
			Vector4 color;
		};

		struct TexCoordSet {
			Vector2 texCoord[4];
		};

		ModelParticleEmitterAdaptor() = default;
		ModelParticleEmitterAdaptor(ModelParticleEmitterAdaptor&&) = default;
		virtual ~ModelParticleEmitterAdaptor() {}

		virtual const std::vector<TexCoordSet>& getTiles() const = 0;

		virtual const std::list<Particle>& getParticles() const = 0;

		virtual void update(size_t animation_index, const AnimationTickArgs& tick, std::vector<ModelBoneAdaptor*>& allbones) = 0;

		virtual const Vector3& getPosition() const = 0;

		virtual const std::vector<uint16_t> getTexture() const = 0;

		constexpr virtual uint16_t getBone() const = 0;

		constexpr virtual uint8_t getParticleType() const = 0;

		constexpr virtual uint32_t getFlags() const = 0;	

		constexpr virtual bool isBillboard() const = 0;

		virtual float lifespanValue(size_t animation_index, const AnimationTickArgs& tick) const = 0;

		virtual std::array<uint16_t, 2> getTextureDimension() const = 0;
		
		constexpr virtual uint8_t getBlendType() const = 0; 
	
		//TODO enum types for particle type, flags, and blend type
	};
};