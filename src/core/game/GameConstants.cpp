#pragma once
#include "../../stdafx.h"
#include <exception>
#include "GameConstants.h"


namespace core {

	const std::string LegacyCharacterCustomization::Name::Skin = "Skin";
	const std::string LegacyCharacterCustomization::Name::Face = "Face";
	const std::string LegacyCharacterCustomization::Name::HairColor = "HairColor";
	const std::string LegacyCharacterCustomization::Name::FacialColor = "FacialColor";
	const std::string LegacyCharacterCustomization::Name::HairStyle = "HairStyle";
	const std::string LegacyCharacterCustomization::Name::FacialStyle = "FacialStyle";

	const std::array<const std::string, 6> LegacyCharacterCustomization::All = {
		LegacyCharacterCustomization::Name::Skin,
		LegacyCharacterCustomization::Name::Face,
		LegacyCharacterCustomization::Name::HairColor,
		LegacyCharacterCustomization::Name::FacialColor,
		LegacyCharacterCustomization::Name::HairStyle,
		LegacyCharacterCustomization::Name::FacialStyle
	};

	const std::map<CharacterSlot, const StackVector<ItemInventorySlotId, 6>> Mapping::CharacterSlotItemInventory = {
		{CharacterSlot::HEAD, {ItemInventorySlotId::HEAD}},
		{CharacterSlot::NECK, {ItemInventorySlotId::NECK}},
		{CharacterSlot::SHOULDER, {ItemInventorySlotId::SHOULDER}},
		{CharacterSlot::SHIRT, {ItemInventorySlotId::SHIRT}},
		{CharacterSlot::CHEST, {
			ItemInventorySlotId::CHEST, 
			ItemInventorySlotId::ROBE
		}},
		{CharacterSlot::BELT, {ItemInventorySlotId::BELT}},
		{CharacterSlot::PANTS, {ItemInventorySlotId::PANTS}},
		{CharacterSlot::BOOTS, {ItemInventorySlotId::BOOTS}},
		{CharacterSlot::BRACERS, {ItemInventorySlotId::BRACERS}},
		{CharacterSlot::GLOVES, {ItemInventorySlotId::GLOVES}},
		{CharacterSlot::HAND_RIGHT, {
			ItemInventorySlotId::RIGHT_HANDED,
			ItemInventorySlotId::GUN,
			ItemInventorySlotId::THROWN,
			ItemInventorySlotId::TWO_HANDED,
			ItemInventorySlotId::ONE_HANDED
		}},
		{CharacterSlot::HAND_LEFT, {
			ItemInventorySlotId::LEFT_HANDED,
			ItemInventorySlotId::BOW,
			ItemInventorySlotId::SHIELD,
			ItemInventorySlotId::TWO_HANDED,
			ItemInventorySlotId::ONE_HANDED,
			ItemInventorySlotId::OFF_HAND
		}},
		{CharacterSlot::CAPE, {ItemInventorySlotId::CAPE}},
		{CharacterSlot::TABARD, {ItemInventorySlotId::TABARD}},
		{CharacterSlot::QUIVER, {ItemInventorySlotId::QUIVER}}
	};

	const std::map<ItemQualityId, QString> Mapping::itemQualityString = {
		{ItemQualityId::POOR, "Poor"},
		{ItemQualityId::NORMAL, "Normal"},
		{ItemQualityId::UNCOMMON, "Uncommon"},
		{ItemQualityId::RARE, "Rare"},
		{ItemQualityId::EPIC, "Epic"},
		{ItemQualityId::LEGENDARY, "Legendary"},
		{ItemQualityId::ARTIFACT, "Artifact"},
		{ItemQualityId::HEIRLOOM, "Heirloom"}
	};

	const std::map<int16_t, QString> Mapping::keyboneNames = {
		{BONE_LARM, "LARM"},
		{BONE_RARM, "RARM"},
		{BONE_LSHOULDER, "LSHOULDER"},	
		{BONE_RSHOULDER, "RSHOULDER"},
		{BONE_STOMACH, "STOMACH"},		
		{BONE_WAIST, "WAIST"},			
		{BONE_HEAD, "HEAD"},			
		{BONE_JAW, "JAW"},			
		{BONE_RFINGER1, "RFINGER1"},		
		{BONE_RFINGER2, "RFINGER2"},		
		{BONE_RFINGER3, "RFINGER3"},		
		{BONE_RFINGERS, "RFINGERS"},		
		{BONE_RTHUMB, "RTHUMB"},		
		{BONE_LFINGER1, "LFINGER1"},		
		{BONE_LFINGER2, "LFINGER2"},		
		{BONE_LFINGER3, "LFINGER3"},		
		{BONE_LFINGERS, "LFINGERS"},		
		{BONE_LTHUMB, "LTHUMB"},		
		{BONE_BTH, "BTH"},			
		{BONE_CSR, "CSR"},			
		{BONE_CSL, "CSL"},			
		{BONE_BREATH, "BREATH"},		
		{BONE_NAME, "NAME"},			
		{BONE_NAMEMOUNT, "NAMEMOUNT"},		
		{BONE_CHD, "CHD"},			
		{BONE_CCH, "CCH"},			
		{BONE_ROOT, "ROOT"},			
		{BONE_WHEEL1, "WHEEL1"},		
		{BONE_WHEEL2, "WHEEL2"},		
		{BONE_WHEEL3, "WHEEL3"},		
		{BONE_WHEEL4, "WHEEL4"},		
		{BONE_WHEEL5, "WHEEL5"},		
		{BONE_WHEEL6, "WHEEL6"},		
		{BONE_WHEEL7, "WHEEL7"},		
		{BONE_WHEEL8, "WHEEL8"}
	};

	const std::map<uint32_t, QString> Mapping::geosetNames = {
		{CG_SKIN_OR_HAIRSTYLE, "Skin/Hair Style"},
		{CG_GEOSET100, "GEO100"},
		{CG_GEOSET200, "GEO200"},
		{CG_GEOSET300, "GEO300"},
		{CG_GLOVES, "Gloves"},
		{CG_BOOTS, "Boots"},
		{CG_TAIL, "Shirt/Tail"},
		{CG_EARS, "Ears"},
		{CG_WRISTBANDS, "Wristbands/Sleeves"},
		{CG_KNEEPADS, "Kneepads/Legcuffs"},
		{CG_CHEST, "Chest"},
		{CG_PANTS, "Pants"},
		{CG_TABARD, "Tabard"},
		{CG_TROUSERS, "Robe/Trousers"},
		{CG_DH_LOINCLOTH, "Loincloth"},
		{CG_CAPE, "Cape"},
		{CG_EYEGLOW, "Eyeglow/Facial Jewelry"},
		{CG_BELT, "Belt"},
		{CG_BONE, "Bone/Skin"},
		{CG_FEET, "Feet/Toes"},
		{CG_GEOSET2100, "GEO2100"},
		{CG_TORSO, "Torso"},
		{CG_HAND_ATTACHMENT, "Hand Attach"},
		{CG_HEAD_ATTACHMENT, "Head Attach"},
		{CG_DH_BLINDFOLDS, "Facewear/Blindfolds"},
		{CG_GEOSET2600, "GEO2600"},
		{CG_GEOSET2700, "GEO2700"},
		{CG_GEOSET2800, "GEO2800"},
		{CG_MECHAGNOME_ARMS_OR_HANDS, "Mechagnome Arms/Hands"},
		{CG_MECHAGNOME_LEGS, "Mechagnome Legs"},
		{CG_MECHAGNOME_FEET, "Mechagnome Feet"},
		{CG_FACE, "Face"},
		{CG_EYES, "Eyes"},
		{CG_EYEBROWS, "Eyebrows"},
		{CG_EARRINGS, "Earrings/Piercings"},
		{CG_NECKLACE, "Necklace"},
		{CG_HEADDRESS, "Headdress"},
		{CG_TAILS, "Tail"},
		{CG_VINES, "Vines/Misc"},
		{CG_TUSKS, "Tusks/Misc"},
		{CG_NOSES, "Nose"},
		{CG_HAIR_DECORATION, "Hair Decoration"},
		{CG_HORN_DECORATION, "Horn Decoration"}
	};

	const std::map<AttachmentPosition, QString> Mapping::attachmentPositionNames = {
		{AttachmentPosition::LEFT_WRIST, "LEFT_WRIST"},
		{AttachmentPosition::RIGHT_PALM, "RIGHT_PALM"},
		{AttachmentPosition::LEFT_PALM, "LEFT_PALM"},
		{AttachmentPosition::RIGHT_ELBOW, "RIGHT_ELBOW"},
		{AttachmentPosition::LEFT_ELBOW, "LEFT_ELBOW"},
		{AttachmentPosition::RIGHT_SHOULDER, "RIGHT_SHOULDER"},
		{AttachmentPosition::LEFT_SHOULDER, "LEFT_SHOULDER"},
		{AttachmentPosition::RIGHT_KNEE, "RIGHT_KNEE"},
		{AttachmentPosition::LEFT_KNEE, "LEFT_KNEE"},
		{AttachmentPosition::RIGHT_HIP, "RIGHT_HIP"},
		{AttachmentPosition::LEFT_HIP, "LEFT_HIP"},
		{AttachmentPosition::HELMET, "HELMET"},
		{AttachmentPosition::BACK, "BACK"},
		{AttachmentPosition::RIGHT_SHOULDER_HORIZONTAL, "RIGHT_SHOULDER_HORIZONTAL"},
		{AttachmentPosition::LEFT_SHOULDER_HORIZONTAL, "LEFT_SHOULDER_HORIZONTAL"},
		{AttachmentPosition::BUST, "BUST"},
		{AttachmentPosition::BUST2, "BUST2"},
		{AttachmentPosition::FACE, "FACE"},
		{AttachmentPosition::ABOVE_CHARACTER, "ABOVE_CHARACTER"},
		{AttachmentPosition::GROUND, "GROUND"},
		{AttachmentPosition::TOP_OF_HEAD, "TOP_OF_HEAD"},
		{AttachmentPosition::LEFT_PALM2, "LEFT_PALM2"},
		{AttachmentPosition::RIGHT_PALM2, "RIGHT_PALM2"},
		{AttachmentPosition::PRE_CAST_2L, "PRE_CAST_2L"},
		{AttachmentPosition::PRE_CAST_2R, "PRE_CAST_2R"},
		{AttachmentPosition::PRE_CAST_3, "PRE_CAST_3"},
		{AttachmentPosition::RIGHT_BACK_SHEATH, "RIGHT_BACK_SHEATH"},
		{AttachmentPosition::LEFT_BACK_SHEATH, "LEFT_BACK_SHEATH"},
		{AttachmentPosition::MIDDLE_BACK_SHEATH, "MIDDLE_BACK_SHEATH"},
		{AttachmentPosition::BELLY, "BELLY"},
		{AttachmentPosition::LEFT_BACK, "LEFT_BACK"},
		{AttachmentPosition::RIGHT_BACK, "RIGHT_BACK"},
		{AttachmentPosition::LEFT_HIP_SHEATH, "LEFT_HIP_SHEATH"},
		{AttachmentPosition::RIGHT_HIP_SHEATH, "RIGHT_HIP_SHEATH"},
		{AttachmentPosition::BUST3, "BUST3"},
		{AttachmentPosition::PALM3, "PALM3"},
		{AttachmentPosition::RIGHT_PALM_UNK2, "RIGHT_PALM_UNK2"},
		{AttachmentPosition::DEMOLISHERVEHICLE, "DEMOLISHERVEHICLE"},
		{AttachmentPosition::DEMOLISHERVEHICLE2, "DEMOLISHERVEHICLE2"},
		{AttachmentPosition::VEHICLE_SEAT1, "VEHICLE_SEAT1"},
		{AttachmentPosition::VEHICLE_SEAT2, "VEHICLE_SEAT2"},
		{AttachmentPosition::VEHICLE_SEAT3, "VEHICLE_SEAT3"},
		{AttachmentPosition::VEHICLE_SEAT4, "VEHICLE_SEAT4"},
		{AttachmentPosition::VEHICLE_SEAT5, "VEHICLE_SEAT5"},
		{AttachmentPosition::VEHICLE_SEAT6, "VEHICLE_SEAT6"},
		{AttachmentPosition::VEHICLE_SEAT7, "VEHICLE_SEAT7"},
		{AttachmentPosition::VEHICLE_SEAT8, "VEHICLE_SEAT8"},
		{AttachmentPosition::LEFT_FOOT, "LEFT_FOOT"},
		{AttachmentPosition::RIGHT_FOOT, "RIGHT_FOOT"},
		{AttachmentPosition::SHIELD_NO_GLOVE, "SHIELD_NO_GLOVE"},
		{AttachmentPosition::SPINELOW, "SPINELOW"},
		{AttachmentPosition::ALTERED_SHOULDER_R, "ALTERED_SHOULDER_R"},
		{AttachmentPosition::ALTERED_SHOULDER_L, "ALTERED_SHOULDER_L"},
		{AttachmentPosition::BELT_BUCKLE, "BELT_BUCKLE"},
		{AttachmentPosition::SHEATH_CROSSBOW, "SHEATH_CROSSBOW"},
		{AttachmentPosition::HEAD_TOP, "HEAD_TOP"}
	};


	std::optional<AttachmentPosition> Mapping::sheathTypeAttachmentPosition(SheathTypes sheath, CharacterSlot slot) {

		switch (sheath) {
		case SheathTypes::SHEATHETYPE_MAINHAND:
			return AttachmentPosition::LEFT_BACK_SHEATH;

		case SheathTypes::SHEATHETYPE_LARGEWEAPON:
			return AttachmentPosition::LEFT_BACK;

		case SheathTypes::SHEATHETYPE_HIPWEAPON:
			return slot == CharacterSlot::HAND_RIGHT ? AttachmentPosition::LEFT_HIP_SHEATH : AttachmentPosition::RIGHT_HIP_SHEATH;

		case SheathTypes::SHEATHETYPE_SHIELD:
			return AttachmentPosition::MIDDLE_BACK_SHEATH;
		}

		return std::nullopt;
	}

}