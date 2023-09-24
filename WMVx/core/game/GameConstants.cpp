#pragma once
#include "../../stdafx.h"
#include <exception>
#include "GameConstants.h"


namespace core {

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