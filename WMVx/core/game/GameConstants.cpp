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

	const std::map<CharacterSlot, const std::vector<ItemInventorySlotId>> Mapping::CharacterSlotItemInventory = {
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


	AttachmentPosition Mapping::sheathTypeAttachmentPosition(SheathTypes sheath, CharacterSlot slot) {

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

		throw std::logic_error("Unsupported sheath type for attachment.");
	}

}