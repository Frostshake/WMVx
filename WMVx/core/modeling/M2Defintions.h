#pragma once
#include <memory>
#include <optional>
#include <variant>
#include <cstdint>
#include <type_traits>
#include "../utility/Vector2.h"
#include "../utility/Vector3.h"
#include "../filesystem/GameFileSystem.h"

namespace core {

	//TODO  move overloads into utility.
	template <class... Fs> struct Overload : Fs... { using Fs::operator()...; };
	template <class... Fs> Overload(Fs...) -> Overload<Fs...>;

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


	enum GlobalFlags : uint32_t {
		TILT_X = 0x1,
		TILT_Y = 0x2,
		UNK_0x4 = 0x4,
		// >= BC
		USE_BLEND_MAP_OVERRIDES = 0x8,
		UNK_0x10 = 0x10,
		// >+ Mists
		LOAD_PHYS_DATA = 0x20,
		UNK_0x40 = 0x40,
		// >= WOD
		UNK_0x80 = 0x80,
		CAMERA_RELATED = 0x100,
		// >= Legion
		NEW_PARTICLE_RCORD = 0x200,
		UNK_0x400 = 0x400,
		TEX_TRANSFORM_USE_BONE_SEQ = 0x800,
		UNK_0x1000 = 0x1000,
		CHUNKED_ANIM_0x2000 = 0x2000,
		UNK_0x4000 = 0x4000,
		UNK_0x8000 = 0x8000,
		UNK_0x10000 = 0x10000,
		UNK_0x20000 = 0x20000,
		UNK_0x40000 = 0x40000,
		UNK_0x80000 = 0x80000,
		UNK_0x100000 = 0x100000,
		UNK_0x200000 = 0x200000,
	};

	using M2Signature = std::array<uint8_t, 4>;

	static bool signatureCompare(M2Signature left, M2Signature right) {
		static_assert(sizeof(M2Signature) == sizeof(uint32_t));
		return *reinterpret_cast<uint32_t*>(left.data()) == *reinterpret_cast<uint32_t*>(right.data());
	}

	//TODO move to better location, include classic expansions.
	enum GameGeneration : uint16_t {
		VANILLA = (1 << 8),
		THE_BURNING_CRUSADE = (2 << 8),
		WRATH_OF_THE_LICH_KING = (3 << 8),
		CATACLYSM = (4 << 8),
		MISTS_OF_PANDARIA = (5 << 8),
		WARLORDS_OF_DRAENOR = (6 << 8),
		LEGION = (7 << 8),
		BATTLE_FOR_AZEROTH = (8 << 8),
		SHADOWLANDS = (9 << 8),
		DRAGONFLIGHT = (10 << 8),
		THE_WAR_WITHIN = (11 << 8)
	};


	namespace Signatures {
		constexpr M2Signature MD20 = { 'M', 'D', '2', '0' };
		constexpr M2Signature MD21 = { 'M', 'D', '2', '1' };
	}
	namespace Signatures {
		constexpr M2Signature AFID = { 'A', 'F', 'I', 'D' };
		constexpr M2Signature SKB1 = { 'S', 'K', 'B', '1' };
		constexpr M2Signature SKID = { 'S', 'K', 'I', 'D' };
		constexpr M2Signature SKPD = { 'S', 'K', 'P', 'D' };
		constexpr M2Signature SKS1 = { 'S', 'K', 'S', '1' };
		constexpr M2Signature SKA1 = { 'S', 'K', 'A', '1' };
		constexpr M2Signature TXID = { 'T', 'X', 'I', 'D' };

		constexpr M2Signature AFSB = { 'A', 'F', 'S', 'B' };
		constexpr M2Signature AFM2 = { 'A', 'F', 'M', '2' };
	}

	class ChunkedFile2 {	//TODO replace old chunked file.
	public:
		struct Chunk {
			M2Signature id;
			uint32_t size;
			size_t offset;	/* not actually part of the file, computed during getChunks*/
		};

		using Chunks = std::map<M2Signature, Chunk>;

		static Chunks getChunks(ArchiveFile* file)
		{
			std::map<M2Signature, Chunk> result;
			auto to_read = file->getFileSize();
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
				to_read -= (header.size + sizeof(header));
			}

			return result;
		}

		explicit ChunkedFile2(std::unique_ptr<ArchiveFile> src) : file(std::move(src)) {
			if (file) {
				chunks = getChunks(file.get());
			}
		}

		bool isChunked() const {
			return chunks.size() > 0;
		}

		std::unique_ptr<ArchiveFile> file;
		Chunks chunks;

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



	template <M2_VER_RANGE R>
	struct ModelRibbonEmitterM2 {
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


	struct FakeAnimationBlockM2 {
		M2Array timestamps;
		M2Array keys;
	};

	template <M2_VER_RANGE R>
	struct ModelParticleParamsM2 {
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

};