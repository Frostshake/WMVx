#pragma once
#include <cstdint>
#include <QString>
#include <map>
#include <vector>

namespace core {

	enum class ItemQualityId : uint32_t {
		POOR = 0,                 //GREY
		NORMAL = 1,                 //WHITE
		UNCOMMON = 2,                 //GREEN
		RARE = 3,                 //BLUE
		EPIC = 4,                 //PURPLE
		LEGENDARY = 5,                 //ORANGE
		ARTIFACT = 6,                 //LIGHT YELLOW
		HEIRLOOM = 7
	};

	enum class CharacterSlot : uint32_t {
		HEAD,
		NECK,
		SHOULDER,
		BOOTS,
		BELT,
		SHIRT,
		PANTS,
		CHEST,
		BRACERS,
		GLOVES,
		HAND_RIGHT,
		HAND_LEFT,
		CAPE,
		TABARD,
		QUIVER,
		MAX
	};


	enum class CharacterRegion : uint32_t {
		ARM_UPPER,
		ARM_LOWER,
		HAND,
		TORSO_UPPER,
		TORSO_LOWER,
		LEG_UPPER,
		LEG_LOWER,
		FOOT,
		ACCESSORY,
		FACE_UPPER,
		FACE_LOWER,
		//TODO ADDITIONAL values?
		//UNKNOWN_11,
		//DH_TATTOOS,
		//CAPE
	};

	enum class ItemInventorySlotId : uint32_t {
		ALL = 0,
		HEAD,
		NECK,
		SHOULDER,
		SHIRT,
		CHEST,
		BELT,
		PANTS,
		BOOTS,
		BRACERS,
		GLOVES,
		RINGS,
		ACCESSORY,
		ONE_HANDED,	
		SHIELD,
		BOW,
		CAPE,
		TWO_HANDED,
		QUIVER,
		TABARD,
		ROBE,
		RIGHT_HANDED, // CLAW
		LEFT_HANDED, // 1HANDED
		OFF_HAND, // HOLDABLE
		AMMO, // unused?
		THROWN,
		GUN,
		UNUSED,
		RELIC,
	};


	//TODO check which versions this is valid for, wow wiki / wmv use different names?
	/*
	Texture Types
	Texture type is 0 for regular textures, nonzero for skinned textures (filename not referenced in the M2 file!) For instance, in the NightElfFemale model, her eye glow is a type 0 texture and has a file name, the other 3 textures have types of 1, 2 and 6. The texture filenames for these come from client database files:
	DBFilesClient\CharSections.dbc
	DBFilesClient\CreatureDisplayInfo.dbc
	DBFilesClient\ItemDisplayInfo.dbc
	(possibly more)
	*/
	enum class TextureType : uint32_t {
		FILENAME = 0,			// Texture given in filename
		BODY,				// Body + clothes
		CAPE,				// Item, Capes ("Item\ObjectComponents\Cape\*.blp")
		ARMOR_REFLECT,		// 
		WEAPON_HANDLE,
		ENVIRONMENT,
		HAIR,				// Hair, bear
		FACIAL_HAIR,
		FUR,				// Tauren fur
		INVENTORY_ART1,		// Used on inventory art M2s (1): inventoryartgeometry.m2 and inventoryartgeometryold.m2
		QUILL,				// Only used in quillboarpinata.m2. I can't even find something referencing that file. Oo Is it used?
		GAMEOBJECT1,		// Skin for creatures or gameobjects #1
		GAMEOBJECT2,		// Skin for creatures or gameobjects #2
		GAMEOBJECT3,		// Skin for creatures or gameobjects #3
		INVENTORY_ART2,		// Used on inventory art M2s (2): ui-buffon.m2 and forcedbackpackitem.m2 (LUA::Model:ReplaceIconTexture("texture"))
	};

	enum BlendMode : uint16_t {
		BM_OPAQUE,
		BM_TRANSPARENT,
		BM_ALPHA_BLEND,
		BM_ADDITIVE,
		BM_ADDITIVE_ALPHA,
		BM_MODULATE,
		BM_MODULATEX2,
		BM_7	//TODO needs a name.
	};

	enum RenderFlags : uint16_t {
		NONE = 0,
		UNLIT = 1,
		UNFOGGED = 2,
		TWO_SIDED = 4,
		BILLBOARD = 8,
		ZBUFFERED = 16
	};

	enum TextureFlag : uint8_t {
		WRAPX = 1,
		WRAPY = 2,
		STATIC = 16
	};

	//TODO copied from WMV, new version may have different constants, tidy
	enum CharacterGeosets : uint32_t {
		CG_HAIRSTYLE,
		CG_GEOSET100,
		CG_GEOSET200,
		CG_GEOSET300,
		CG_GLOVES,
		CG_BOOTS, // 5
		CG_EARS = 7,
		CG_WRISTBANDS,
		CG_KNEEPADS,
		CG_PANTS, // 10
		CG_PANTS2,
		CG_TABARD,
		CG_TROUSERS,
		CG_CAPE = 15, // 15
		CG_EYEGLOW = 17,
		CG_BELT,
		MAX
	};

	//TODO copied from WMV, compare with old and new versions (POSITION_SLOTS), tidy
	enum class AttachmentPosition : uint32_t {
		LEFT_WRIST = 0, // Mountpoint
		RIGHT_PALM,
		LEFT_PALM,
		RIGHT_ELBOW,
		LEFT_ELBOW,
		RIGHT_SHOULDER, // 5
		LEFT_SHOULDER,
		RIGHT_KNEE,
		LEFT_KNEE,
		RIGHT_HIP,
		LEFT_HIP, // 10
		HELMET,
		BACK,
		RIGHT_SHOULDER_HORIZONTAL,
		LEFT_SHOULDER_HORIZONTAL,
		BUST, // 15
		BUST2,
		FACE,
		ABOVE_CHARACTER,
		GROUND,
		TOP_OF_HEAD, // 20
		LEFT_PALM2,
		RIGHT_PALM2,
		PRE_CAST_2L,
		PRE_CAST_2R,
		PRE_CAST_3, // 25
		RIGHT_BACK_SHEATH,
		LEFT_BACK_SHEATH,
		MIDDLE_BACK_SHEATH,
		BELLY,
		LEFT_BACK, // 30
		RIGHT_BACK,
		LEFT_HIP_SHEATH,
		RIGHT_HIP_SHEATH,
		BUST3, // Spell Impact
		PALM3, // 35
		RIGHT_PALM_UNK2,
		DEMOLISHERVEHICLE,
		DEMOLISHERVEHICLE2,
		VEHICLE_SEAT1,
		VEHICLE_SEAT2, // 40
		VEHICLE_SEAT3,
		VEHICLE_SEAT4
	};

	//TODO copied from WMV, check against newer versions
	enum SheathTypes
	{
		SHEATHETYPE_NONE = 0,
		SHEATHETYPE_MAINHAND = 1,
		SHEATHETYPE_LARGEWEAPON = 2,
		SHEATHETYPE_HIPWEAPON = 3,
		SHEATHETYPE_SHIELD = 4
	};

	//TODO add values from modern wow versions
	enum class CharacterSectionType : uint32_t {
		Skin = 0,
		Face = 1,
		FacialHair = 2,
		Hair = 3,
		Underwear = 4
	};

	enum class Gender : uint32_t {
		MALE = 0,
		FEMALE = 1
	};

	enum KeyBones : int32_t {
		BONE_LARM = 0,		// 0, ArmL: Left upper arm
		BONE_RARM,			// 1, ArmR: Right upper arm
		BONE_LSHOULDER,		// 2, ShoulderL: Left Shoulder / deltoid area
		BONE_RSHOULDER,		// 3, ShoulderR: Right Shoulder / deltoid area
		BONE_STOMACH,		// 4, SpineLow: (upper?) abdomen
		BONE_WAIST,			// 5, Waist: (lower abdomen?) waist
		BONE_HEAD,			// 6, Head
		BONE_JAW,			// 7, Jaw: jaw/mouth
		BONE_RFINGER1,		// 8, IndexFingerR: (Trolls have 3 "fingers", this points to the 2nd one.
		BONE_RFINGER2,		// 9, MiddleFingerR: center finger - only used by dwarfs.. don't know why
		BONE_RFINGER3,		// 10, PinkyFingerR: (Trolls have 3 "fingers", this points to the 3rd one.
		BONE_RFINGERS,		// 11, RingFingerR: Right fingers -- this is -1 for trolls, they have no fingers, only the 3 thumb like thingys
		BONE_RTHUMB,		// 12, ThumbR: Right Thumb
		BONE_LFINGER1,		// 13, IndexFingerL: (Trolls have 3 "fingers", this points to the 2nd one.
		BONE_LFINGER2,		// 14, MiddleFingerL: Center finger - only used by dwarfs.
		BONE_LFINGER3,		// 15, PinkyFingerL: (Trolls have 3 "fingers", this points to the 3rd one.
		BONE_LFINGERS,		// 16, RingFingerL: Left fingers
		BONE_LTHUMB,		// 17, ThubbL: Left Thumb
		BONE_BTH,			// 18, $BTH: In front of head
		BONE_CSR,			// 19, $CSR: Left hand
		BONE_CSL,			// 20, $CSL: Left hand
		BONE_BREATH,		// 21, _Breath
		BONE_NAME,			// 22, _Name
		BONE_NAMEMOUNT,		// 23, _NameMount
		BONE_CHD,			// 24, $CHD: Head
		BONE_CCH,			// 25, $CCH: Bust
		BONE_ROOT,			// 26, Root: The "Root" bone,  this controls rotations, transformations, etc of the whole model and all subsequent bones.
		BONE_WHEEL1,		// 27, Wheel1
		BONE_WHEEL2,		// 28, Wheel2
		BONE_WHEEL3,		// 29, Wheel3
		BONE_WHEEL4,		// 30, Wheel4
		BONE_WHEEL5,		// 31, Wheel5
		BONE_WHEEL6,		// 32, Wheel6
		BONE_WHEEL7,		// 33, Wheel7
		BONE_WHEEL8,		// 34, Wheel8
		BONE_MAX
	};

	enum ModelBoneFlags : uint32_t {
		ignoreParentTranslate = 0x1,
		ignoreParentScale = 0x2,
		ignoreParentRotation = 0x4,
		spherical_billboard = 0x8,
		cylindrical_billboard_lock_x = 0x10,
		cylindrical_billboard_lock_y = 0x20,
		cylindrical_billboard_lock_z = 0x40,
		transformed = 0x200,
		kinematic_bone = 0x400,       // MoP+: allow physics to influence this bone
		helmet_anim_scaled = 0x1000,  // set blend_modificator to helmetAnimScalingRec.m_amount for this bone
		something_sequence_id = 0x2000, // <=bfa+, parent_bone+submesh_id are a sequence id instead?!
	};

	enum ParticleEmitterType : uint8_t {
		PLANE = 1,
		SPHERE = 2,
		SPLINE = 3
	};

	//TODO MATCH wiki
	enum ModelParticleFlags : uint32_t {
		LIGHTING =			0x1,		// Particles are affected by lighting;
		EMISSIONORIENTATION = 0x4,  // On emission, particle orientation is affected by player orientation
		WORLDSPACE =    0x8,        // Particles travel "up" in world space, rather than model
		DONOTTRAIL =    0x10,       // Do not trail 
		UNLIGHTING =			0x20,		// Unlightning
		USEBURST =		0x40,		// Use Burst Multiplier 
		MODELSPACE =    0x80,       // Particles in model space
		RANDOMSPAWN =   0x200,		// spawn position randomized in some way?
		PINNED =        0x400,      // Pinned Particles, their quad enlarges from their creation
				// position to where they expand
		DONOTBILLBOARD = 0x1000,     // Wiki says: XYQuad Particles. They align to XY axis
															  // facing Z axis direction
		RANDOMTEXTURE =  0x10000,    // Choose Random Texture
		OUTWARD =        0x20000,    // "Outward" particles, most emitters have this and
															  // their particles move away from the origin, when they
															  // don't the particles start at origin+(speed*life) and
															  // move towards the origin
		RANDOMSTART =    0x200000,   // Random Flip Book Start
		BONEGENERATOR =  0x1000000,  // Bone generator = bone, not joint
		DONOTTHROTTLE =  0x4000000,  // Do not throttle emission rate based on distance
		MULTITEXTURE =   0x10000000, // Particle uses multi-texturing. This affects emitter values
	};

	class CharacterUtil {
	public:
		static bool slotHasModel(CharacterSlot slot)
		{
			return slot == CharacterSlot::HEAD ||
				slot == CharacterSlot::SHOULDER ||
				slot == CharacterSlot::HAND_RIGHT ||
				slot == CharacterSlot::HAND_LEFT ||
				slot == CharacterSlot::QUIVER;
		}
	};

	class GenderUtil {
	public:
		static inline QString toString(Gender value) {
			if (value == Gender::FEMALE) {
				return "Female";
			}

			return "Male";
		}

		static inline char toChar(Gender value) {
			if (value == Gender::FEMALE) {
				return 'F';
			}

			return 'M';
		}

		static inline Gender fromString(QString str) {
			if (str.compare("Female", Qt::CaseInsensitive) == 0) {
				return Gender::FEMALE;
			}

			return Gender::MALE;
		}
	private:
		GenderUtil() = delete;
	};

	class Mapping {
	public:
		static const std::map<CharacterSlot, const std::vector<ItemInventorySlotId>> CharacterSlotItemInventory;
		static const std::map<ItemQualityId, QString> itemQualityString;
		static AttachmentPosition sheathTypeAttachmentPosition(SheathTypes, CharacterSlot);
	};
};