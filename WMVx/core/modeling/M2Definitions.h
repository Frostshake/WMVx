#pragma once
#include <memory>
#include <optional>
#include <variant>
#include <cstdint>
#include <type_traits>
#include "../utility/Vector2.h"
#include "../utility/Vector3.h"
#include "../filesystem/GameFileSystem.h"
#include "../utility/Color.h"
#include "../game/GameConstants.h"

namespace core {

	constexpr uint32_t M2_VER_MIN = 0;
	constexpr uint32_t M2_VER_MAX = UINT32_MAX;

	constexpr uint32_t M2_VER_ALPHA = 256;
	constexpr uint32_t M2_VER_VANILLA_MIN = 256;
	constexpr uint32_t M2_VER_VANILLA_MAX = 257;
	constexpr uint32_t M2_VER_TBC_MIN = 260;
	constexpr uint32_t M2_VER_TBC_MAX = 263;
	constexpr uint32_t M2_VER_WOTLK = 264;
	constexpr uint32_t M2_VER_CATA_MIN = 265;
	constexpr uint32_t M2_VER_CATA_MAX = 272;
	constexpr uint32_t M2_VER_MOP_WOD = 272;
	constexpr uint32_t M2_VER_LEGION_PLUS = 272;


	struct M2_VER_RANGE {
	public:
		constexpr M2_VER_RANGE(uint32_t min, uint32_t max) : 
			MIN(min), MAX(max) {}

		explicit constexpr M2_VER_RANGE(uint32_t v) : MIN(v), MAX(v) {}

		static constexpr M2_VER_RANGE EXACT(uint32_t v) {
			return M2_VER_RANGE(v, v);
		}

		static constexpr M2_VER_RANGE UPTO(uint32_t max) {
			return M2_VER_RANGE(M2_VER_MIN, max);
		}

		static constexpr M2_VER_RANGE FROM(uint32_t min) {
			return M2_VER_RANGE(min, M2_VER_MAX);
		}

		uint32_t MIN;
		uint32_t MAX;
	};

	template<M2_VER_RANGE R>
	struct M2_VER_CONDITION {

		constexpr static bool eval(M2_VER_RANGE Test) {
			return R.MIN <= Test.MIN && R.MAX >= Test.MAX;
		}

		constexpr static bool eval(uint32_t Test) {
			return R.MIN <= Test && R.MAX >= Test;
		}

		template< M2_VER_RANGE Test>
		using enable_if = std::enable_if<eval(Test)>::type;

		constexpr static M2_VER_RANGE Range = R;
	};

	template<uint32_t V>
	using M2_VER_CONDITION_AFTER = M2_VER_CONDITION<M2_VER_RANGE::FROM(V)>;

	template<uint32_t V>
	using M2_VER_CONDITION_BEFORE = M2_VER_CONDITION<M2_VER_RANGE::UPTO(V)>;

	template< uint32_t MIN, uint32_t MAX>
	using M2_VER_CONDITION_BETWEEN = M2_VER_CONDITION<M2_VER_RANGE(MIN, MAX)>;

	template<M2_VER_RANGE... Versions>
	struct M2_VER_RANGE_LIST {

		static bool match(uint32_t version, auto match_fn) {
			bool handled = false;

			auto each = [&]<M2_VER_RANGE T>() -> bool {
				assert(!handled);

				if (M2_VER_CONDITION<T>::eval(version)) {
					match_fn.template operator()<T>();
					handled = true;
					
					return true;
				}

				return false;
			};
			//TODO confirm this does exist early.
			(each.operator()<Versions>() || ...);

			return handled;
		}

	};

	using M2Signature = std::array<uint8_t, 4>;

	static bool signatureCompare(M2Signature left, M2Signature right) {
		static_assert(sizeof(M2Signature) == sizeof(uint32_t));
		return *reinterpret_cast<uint32_t*>(left.data()) == *reinterpret_cast<uint32_t*>(right.data());
	}

	namespace Signatures {
		constexpr M2Signature MD20 = { 'M', 'D', '2', '0' };
		constexpr M2Signature MD21 = { 'M', 'D', '2', '1' };
		constexpr M2Signature SKIN = { 'S', 'K', 'I', 'N' };
	}

	namespace Signatures {
		constexpr M2Signature AFID = { 'A', 'F', 'I', 'D' };
		constexpr M2Signature SKB1 = { 'S', 'K', 'B', '1' };
		constexpr M2Signature SKID = { 'S', 'K', 'I', 'D' };
		constexpr M2Signature SFID = { 'S', 'F', 'I', 'D' };
		constexpr M2Signature SKPD = { 'S', 'K', 'P', 'D' };
		constexpr M2Signature SKS1 = { 'S', 'K', 'S', '1' };
		constexpr M2Signature SKA1 = { 'S', 'K', 'A', '1' };
		constexpr M2Signature TXID = { 'T', 'X', 'I', 'D' };

		constexpr M2Signature AFSB = { 'A', 'F', 'S', 'B' };
		constexpr M2Signature AFM2 = { 'A', 'F', 'M', '2' };
	}

	class ChunkedFile {
	public:
		struct Chunk {
			M2Signature id;
			uint32_t size;
			size_t offset;	/* not actually part of the file, computed during getChunks*/
		};

		using Chunks = std::map<M2Signature, Chunk>;

		static Chunks getChunks(ArchiveFile* file)
		{
			//TODO need to be able to detect if file is chunked or not ahead of time, to avoid back chunks.
			std::map<M2Signature, Chunk> result;
			size_t to_read = file->getFileSize();
			size_t offset = 0;
			struct {
				M2Signature id;
				uint32_t size;
			} header;

			while (to_read > sizeof(header)) {
				file->read(&header, sizeof(header), offset);
				offset += sizeof(header);

				assert(!result.contains(header.id));
				result.emplace(header.id, Chunk{
						header.id,
						header.size,
						offset
					});

				offset += header.size;
				const auto move = header.size + sizeof(header);
				//assert(to_read <= move);
				to_read -= std::min(to_read, move);
			}

			return result;
		}

		explicit ChunkedFile(std::unique_ptr<ArchiveFile> src, bool load = true) : file(std::move(src)) {
			if (file &&  load) {
				chunks = getChunks(file.get());
			}
		}

		bool isChunked() const {
			return chunks.size() > 0;
		}

		std::unique_ptr<ArchiveFile> file;
		Chunks chunks;

	};

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



	constexpr uint16_t TEXTURE_MAX = 32;

	struct M2Array {
		uint32_t size;
		uint32_t offset;
	};

	struct M2Box {
		Vector3 min;
		Vector3 max;
	};

	namespace Chunks {

		struct AFID {
			uint16_t animationId;
			uint16_t variationId;
			uint32_t fileId;
		};

		struct SKB1 {
			M2Array bones;
			M2Array keyBoneLookup;
		};

		struct SKID {
		public:
			uint32_t skeletonFileId;
		};

		struct SKPD {
			std::array<uint8_t, 8> unknown;
			uint32_t parentSkelFileId;
			std::array<uint8_t, 4> unknown2;
		};

		struct SKS1 {
			M2Array globalSequences;
			M2Array animations;
			M2Array animationLookup;
		};

		struct SKA1 {
			M2Array attachments;
			M2Array attachmentLookup;
		};

		struct TXID {
			uint32_t fileDataId;
		};
	}


	struct ModelTextureM2 {
		uint32_t type;
		uint32_t flags;
		M2Array name;
	};
	struct ModelVertexM2 {

		constexpr static uint8_t BONE_COUNT = 4;

		Vector3 position;
		uint8_t boneWeights[BONE_COUNT];
		uint8_t bones[BONE_COUNT];
		Vector3 normal;
		Vector2 textureCoords;
		float unknown1;	//TODO check values, wiki has these 2 floats as a secondry textureCoords vector
		float unknown2;
	};

	struct ModelRenderFlagsM2 {
		uint16_t flags;
		uint16_t blend;
	};

	// texture unit or batch
	struct ModelTextureUnitM2 {
		uint16_t flags;	//TODO wow dev wiki has this as two int8's check why
		uint16_t sharderId;
		uint16_t submeshIndex;
		uint16_t submeshIndex2;
		int16_t colorIndex;
		uint16_t renderFlagsIndex;
		uint16_t textureUnitIndex;
		uint16_t mode;
		uint16_t textureId;
		uint16_t textureUnit2;
		uint16_t transparencyIndex;
		uint16_t textureAnimationId;
	};

	struct AnimationBlockHeader {
		uint32_t size;
		uint32_t offset;
	};

	struct CharacterRegionCoords {
		int32_t positionX;
		int32_t positionY;
		int32_t sizeX;
		int32_t sizeY;
	};



	template <M2_VER_RANGE R, typename T = void>
	struct AnimationBlockM2;

	template <M2_VER_RANGE R>
	struct AnimationBlockM2<R, M2_VER_CONDITION_AFTER<M2_VER_WOTLK>::enable_if<R>> {
		uint16_t interpolationType;
		int16_t globalSequence;
		M2Array timestamps;
		M2Array keys;
	};

	template <M2_VER_RANGE R>
	struct AnimationBlockM2<R, M2_VER_CONDITION_BEFORE<M2_VER_WOTLK - 1>::enable_if<R>> {
		uint16_t interpolationType;
		int16_t globalSequence;
		M2Array ranges;
		M2Array timestamps;
		M2Array keys;
	};

	template <M2_VER_RANGE R>
	struct TextureAnimationM2 {
		AnimationBlockM2<R> translation;
		AnimationBlockM2<R> rotation;
		AnimationBlockM2<R> scale;
	};

	template <M2_VER_RANGE R>
	struct ModelColorM2 {
		AnimationBlockM2<R> color;
		AnimationBlockM2<R> opacity;
	};

	template <M2_VER_RANGE R>
	struct ModelTransparencyM2 {
		AnimationBlockM2<R> transparency;
	};

	template <M2_VER_RANGE R>
	struct ModelAttachmentM2 {
		uint32_t id;
		uint16_t bone;
		uint16_t unknown1;
		Vector3 position;
		AnimationBlockM2<R> unknown2;
	};

	template <M2_VER_RANGE R, typename T = void>
	struct ModelBoneM2;


	template <M2_VER_RANGE R>
	struct ModelBoneM2<R, M2_VER_CONDITION_AFTER<M2_VER_TBC_MIN>::enable_if<R>> {
		int32_t keyBoneId;
		uint32_t flags;
		int16_t parentBoneId;
		uint16_t submeshId;
		uint32_t unknown1;
		AnimationBlockM2<R> translation;
		AnimationBlockM2<R> rotation;
		AnimationBlockM2<R> scale;
		Vector3 pivot;
	};

	template <M2_VER_RANGE R>
	struct ModelBoneM2<R, M2_VER_CONDITION_BEFORE<M2_VER_TBC_MIN - 1>::enable_if<R>> {
		int32_t keyBoneId;
		uint32_t flags;
		int16_t parentBoneId;
		uint16_t submeshId;
		AnimationBlockM2<R> translation;
		AnimationBlockM2<R> rotation;
		AnimationBlockM2<R> scale;
		Vector3 pivot;
	};

	template <M2_VER_RANGE R, typename T = void>
	struct ModelRibbonEmitterM2;

	template <M2_VER_RANGE R>
	struct ModelRibbonEmitterM2<R, M2_VER_CONDITION_AFTER<M2_VER_WOTLK>::enable_if<R>> {
		uint32_t id;
		uint32_t boneIndex;
		Vector3 position;
		M2Array textures;
		M2Array materials;
		AnimationBlockM2<R> color;
		AnimationBlockM2<R> alpha;
		AnimationBlockM2<R> heightAbove;
		AnimationBlockM2<R> heightBelow;
		float edgesPerSecond;
		float edgeLifetime;
		float gravity;
		uint16_t textureRows;
		uint16_t textureCols;
		AnimationBlockM2<R> textureSlot;
		AnimationBlockM2<R> visibility;
		int16_t priorityPlane;
		int8_t ribbonColorIndex;
		int8_t textureTransformLookupIndex;
	};

	template <M2_VER_RANGE R>
	struct ModelRibbonEmitterM2<R, M2_VER_CONDITION_BEFORE<M2_VER_WOTLK - 1>::enable_if<R>> {
		uint32_t id;
		uint32_t boneIndex;
		Vector3 position;
		M2Array textures;
		M2Array materials;
		AnimationBlockM2<R> color;
		AnimationBlockM2<R> alpha;
		AnimationBlockM2<R> heightAbove;
		AnimationBlockM2<R> heightBelow;
		float edgesPerSecond;
		float edgeLifetime;
		float gravity;
		uint16_t textureRows;
		uint16_t textureCols;
		AnimationBlockM2<R> textureSlot;
		AnimationBlockM2<R> visibility;
	};


	struct FakeAnimationBlockM2 {
		M2Array timestamps;
		M2Array keys;
	};

	template <M2_VER_RANGE R>
	struct ModelParticleParamsM2 {
		// TODO remove 
		FakeAnimationBlockM2 color;
		FakeAnimationBlockM2 opacity;
		FakeAnimationBlockM2 scale;
		Vector2 scaleVary;
		FakeAnimationBlockM2 headCellTrack;
		FakeAnimationBlockM2 tailCellTrack;
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



	template <M2_VER_RANGE R, typename T = void>
	struct AnimationSequenceM2;

	template <M2_VER_RANGE R>
	struct AnimationSequenceM2<R, M2_VER_CONDITION_AFTER<M2_VER_WOTLK>::enable_if<R>> {
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

	template <M2_VER_RANGE R>
	struct AnimationSequenceM2<R, M2_VER_CONDITION_BEFORE<M2_VER_WOTLK - 1>::enable_if<R>> {
		uint16_t id;
		uint16_t variationId;
		uint32_t startTimestamp;
		uint32_t endTimestamp;
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


	template <M2_VER_RANGE R, typename T = void>
	struct ModelViewM2;

	template<M2_VER_RANGE R>
	struct ModelViewM2<R, M2_VER_CONDITION_AFTER<M2_VER_CATA_MIN>::enable_if<R>> {
		uint8_t id[4];
		M2Array indices;
		M2Array triangles;
		M2Array properties;
		M2Array submeshes;
		M2Array textureUnits;
		uint32_t boneCountMax;
		M2Array shadowBatches;
	};

	template<M2_VER_RANGE R>
	struct ModelViewM2<R, M2_VER_CONDITION_BETWEEN<M2_VER_WOTLK, M2_VER_CATA_MIN - 1>::enable_if<R>> {
		uint8_t id[4];
		M2Array indices;
		M2Array triangles;
		M2Array properties;
		M2Array submeshes;
		M2Array textureUnits;
		uint32_t boneCountMax;
	};

	template<M2_VER_RANGE R>
	struct ModelViewM2<R, M2_VER_CONDITION_BEFORE<M2_VER_WOTLK - 1>::enable_if<R>> {
		M2Array indices;
		M2Array triangles;
		M2Array properties;
		M2Array submeshes;
		M2Array textureUnits;
		uint32_t boneCountMax;
	};


	template <M2_VER_RANGE R, typename T = void>
	struct ModelGeosetM2;


	template <M2_VER_RANGE R>
	struct ModelGeosetM2<R, M2_VER_CONDITION_AFTER<M2_VER_TBC_MIN>::enable_if<R>> {
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

	template <M2_VER_RANGE R>
	struct ModelGeosetM2<R, M2_VER_CONDITION_BEFORE<M2_VER_TBC_MIN - 1>::enable_if<R>> {
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
	};


	template<M2_VER_RANGE R, typename T = void>
	struct ModelParticleEmitterM2;

	template <M2_VER_RANGE R>
	struct ModelParticleEmitterM2<R, M2_VER_CONDITION_AFTER<M2_VER_CATA_MIN>::enable_if<R>> {
		uint32_t id;
		uint32_t flags;
		Vector3 position;
		uint16_t bone;
		struct                                  // For multi-textured particles actually three ids
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
		uint16_t textureDimensionRows;
		uint16_t textureDimensionColumns;
		AnimationBlockM2<R> emissionSpeed;
		AnimationBlockM2<R> speedVariation;
		AnimationBlockM2<R> verticalRange;
		AnimationBlockM2<R> horizontalRange;
		AnimationBlockM2<R> gravity;
		AnimationBlockM2<R> lifespan;

		float lifespanVary;

		AnimationBlockM2<R> emissionRate;

		float emissionRateVary;
		
		AnimationBlockM2<R> emissionAreaLength;
		AnimationBlockM2<R> emissionAreaWidth;
		AnimationBlockM2<R> zSource;

		FakeAnimationBlockM2 color;
		FakeAnimationBlockM2 opacity;
		FakeAnimationBlockM2 scale;
		Vector2 scaleVary;
		FakeAnimationBlockM2 headCellTrack;
		FakeAnimationBlockM2 tailCellTrack;

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

		M2Box tumble;
		Vector3 windVector;
		float windTime;
		float followSpeed1;
		float followScale1;
		float followSpeed2;
		float followScale2;
		M2Array splinePoints;
		AnimationBlockM2<R> enabledIn;

		Vector2FP69 multiTextureParam0[2];
		Vector2FP69 multiTextureParam1[2];
	};

	template <M2_VER_RANGE R>
	struct ModelParticleEmitterM2<R, M2_VER_CONDITION_BETWEEN<M2_VER_WOTLK, M2_VER_CATA_MIN - 1>::enable_if<R>> {
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
		uint16_t textureDimensionRows;
		uint16_t textureDimensionColumns;
		AnimationBlockM2<R> emissionSpeed;
		AnimationBlockM2<R> speedVariation;
		AnimationBlockM2<R> verticalRange;
		AnimationBlockM2<R> horizontalRange;
		AnimationBlockM2<R> gravity;
		AnimationBlockM2<R> lifespan;

		float lifespanVary;

		AnimationBlockM2<R> emissionRate;

		float emissionRateVary;

		AnimationBlockM2<R> emissionAreaLength;
		AnimationBlockM2<R> emissionAreaWidth;
		AnimationBlockM2<R> zSource;

		FakeAnimationBlockM2 color;
		FakeAnimationBlockM2 opacity;
		FakeAnimationBlockM2 scale;
		Vector2 scaleVary;
		FakeAnimationBlockM2 headCellTrack;
		FakeAnimationBlockM2 tailCellTrack;

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

		M2Box tumble;
		Vector3 windVector;
		float windTime;
		float followSpeed1;
		float followScale1;
		float followSpeed2;
		float followScale2;
		M2Array splinePoints;
		AnimationBlockM2<R> enabledIn;
	};

	template <M2_VER_RANGE R>
	struct ModelParticleEmitterM2<R, M2_VER_CONDITION_BETWEEN<M2_VER_TBC_MAX, M2_VER_WOTLK - 1>::enable_if<R>> {
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
		uint16_t textureDimensionRows;
		uint16_t textureDimensionColumns;
		AnimationBlockM2<R> emissionSpeed;
		AnimationBlockM2<R> speedVariation;
		AnimationBlockM2<R> verticalRange;
		AnimationBlockM2<R> horizontalRange;
		AnimationBlockM2<R> gravity;
		AnimationBlockM2<R> lifespan;

		AnimationBlockM2<R> emissionRate;

		AnimationBlockM2<R> emissionAreaLength;
		AnimationBlockM2<R> emissionAreaWidth;
		AnimationBlockM2<R> zSource;

		float midPoint;
		ColorRGBA<uint8_t> colorValues[3]; //CImVector
		float scalesValues[3];
		uint16_t lifespanUVAnim[3];
		uint16_t decayUVAnim[3];
		uint16_t tailUVAnim[2];
		uint16_t tailDecayUVAnim[2];

		float tailLength;
		float twinkleSpeed;
		float twinklePercent;
		float twinkleScaleMin;
		float twinkleScaleMax;
		float burstMultiplier;
		float drag;

		float spin;

		M2Box tumble;
		Vector3 windVector;
		float windTime;
		float followSpeed1;
		float followScale1;
		float followSpeed2;
		float followScale2;
		M2Array splinePoints;
		AnimationBlockM2<R> enabledIn;
	};

	template <M2_VER_RANGE R>
	struct ModelParticleEmitterM2<R, M2_VER_CONDITION_BEFORE<M2_VER_TBC_MAX -1>::enable_if<R>> {
		uint32_t id;
		uint32_t flags;
		Vector3 position;
		uint16_t bone;
		uint16_t texture;
		M2Array geometryModelFilename;
		M2Array recursionModelFilename;

		uint16_t blendType;
		uint16_t emitterType;

		uint8_t particleType;
		uint8_t headTail;

		uint16_t textureTileRotation;
		uint16_t textureDimensionRows;
		uint16_t textureDimensionColumns;
		AnimationBlockM2<R> emissionSpeed;
		AnimationBlockM2<R> speedVariation;
		AnimationBlockM2<R> verticalRange;
		AnimationBlockM2<R> horizontalRange;
		AnimationBlockM2<R> gravity;
		AnimationBlockM2<R> lifespan;

		AnimationBlockM2<R> emissionRate;

		AnimationBlockM2<R> emissionAreaLength;
		AnimationBlockM2<R> emissionAreaWidth;
		AnimationBlockM2<R> zSource;

		float midPoint;
		ColorRGBA<uint8_t> colorValues[3]; //CImVector
		float scalesValues[3];
		uint16_t lifespanUVAnim[3];
		uint16_t decayUVAnim[3];
		uint16_t tailUVAnim[2];
		uint16_t tailDecayUVAnim[2];

		float tailLength;
		float twinkleSpeed;
		float twinklePercent;
		float twinkleScaleMin;
		float twinkleScaleMax;
		float burstMultiplier;
		float drag;

		float spin;

		M2Box tumble;
		Vector3 windVector;
		float windTime;//
		float followSpeed1;
		float followScale1;
		float followSpeed2;
		float followScale2;
		M2Array splinePoints;
		AnimationBlockM2<R> enabledIn;

		//TODO compare this with old WMV source (vanilla), they look to have slightly different fields (investigation needed)
	};

	

};