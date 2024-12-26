#include "../../stdafx.h"
#include "AttachmentCustomization.h"
#include "Model.h"
#include "MergedModel.h"
#include "Scene.h"

namespace core {
	StackVector<AttachmentPosition, 2> StandardAttachmentCustomizationProvider::getAttachmentPositions(CharacterSlot slot, const ItemRecordAdaptor* item, bool sheatheWeapons) const
	{
		StackVector<AttachmentPosition, 2> attach_positions;

		switch (slot) {
		case CharacterSlot::HEAD:
			attach_positions = { AttachmentPosition::HELMET };
			break;
		case CharacterSlot::SHOULDER:
			attach_positions = {
				AttachmentPosition::LEFT_SHOULDER,
				AttachmentPosition::RIGHT_SHOULDER,
			};
			break;
		case CharacterSlot::HAND_LEFT:
			attach_positions = { AttachmentPosition::LEFT_PALM };
			break;
		case CharacterSlot::HAND_RIGHT:
			attach_positions = { AttachmentPosition::RIGHT_PALM };
			break;
		case CharacterSlot::QUIVER:
			attach_positions = { AttachmentPosition::RIGHT_BACK_SHEATH };
			break;
		case CharacterSlot::BELT: //DF+, fixed position
			attach_positions = {
				AttachmentPosition::BELT_BUCKLE
			};
			break;
		}

		if (slot == CharacterSlot::HAND_LEFT || slot == CharacterSlot::HAND_RIGHT) {
			if (item->getInventorySlotId() == ItemInventorySlotId::SHIELD) {
				attach_positions = { AttachmentPosition::LEFT_WRIST };
			}

			auto sheath_type = (SheathTypes)item->getSheatheTypeId();
			if (sheatheWeapons && sheath_type > SheathTypes::SHEATHETYPE_NONE) {
				auto result = Mapping::sheathTypeAttachmentPosition(sheath_type, slot);
				if (result.has_value()) {
					attach_positions = {
						result.value()
					};
				}
			}
		}

		return attach_positions;
	}

	std::unique_ptr<Attachment> StandardAttachmentCustomizationProvider::makeAttachment(
		CharacterSlot slot,
		AttachmentPosition position,
		const CharacterItemWrapper& wrapper,
		const GameFileUri& model_file,
		const GameFileUri& texture_file,
		Model* parent,
		Scene* scene
	) const
	{
		auto att = std::make_unique<Attachment>(slot);
		att->setPosition(position, 0, Vector3());

		att->visit<Attachment::AttachOwnedModel>([&](Attachment::AttachOwnedModel* owned) {

			{
				auto [m2_ptr, tex_info] = modelFactory(gameFS, model_file);
				owned->model = std::move(m2_ptr);

				for (auto& tex : tex_info) {
					owned->loadTexture(owned->model.get(), tex.index, tex.defintion, tex.uri, scene->textureManager, gameFS);
				}
			}

			owned->initAnimationData(owned->model.get());
			owned->initGeosetData(owned->model.get(), true);

			//load attachment texture
			auto tex = scene->textureManager.add(texture_file, gameFS);
			if (tex != nullptr) {
				owned->replacableTextures[TextureType::CAPE] = tex;
			}
		});

		
		parent->setAttachmentPosition(att.get(), position);

		return std::move(att);
	}

	StackVector<AttachmentPosition, 2> MergedAwareAttachmentCustomizationProvider::getAttachmentPositions(CharacterSlot slot, const ItemRecordAdaptor* item, bool sheatheWeapons) const
	{
		auto attach_positions = StandardAttachmentCustomizationProvider::getAttachmentPositions(slot, item, sheatheWeapons);
		
		switch (slot) {
		case CharacterSlot::CHEST:
		case CharacterSlot::SHIRT:
		case CharacterSlot::GLOVES:
		case CharacterSlot::BOOTS:
		case CharacterSlot::PANTS:
			attach_positions = {
				AttachmentPosition::GROUND
			};
			break;
		}

		return attach_positions;
	}

	std::unique_ptr<Attachment> MergedAwareAttachmentCustomizationProvider::makeAttachment(
		CharacterSlot slot, 
		AttachmentPosition position, 
		const CharacterItemWrapper& wrapper,
		const GameFileUri& model_file, 
		const GameFileUri& texture_file,
		Model* parent,
		Scene* scene
	) const
	{

		const bool is_merged_type = isMergedType(slot);

		if (is_merged_type) {
			const auto display_id = wrapper.display()->getId();

			// enforce some assumptions on sizes.
			static_assert(sizeof(display_id) == sizeof(uint32_t));
			static_assert(sizeof(MergedModel::id_t) == sizeof(uint64_t));
			const MergedModel::id_t merged_id = (uint64_t(display_id) << 32) + (uint64_t(slot) << 16) + uint64_t(position);

			auto custom = std::make_unique<MergedModel>(
				parent,
				MergedModel::Type::CHAR_ATTACHMENT_ADDITION,
				merged_id
			);

			assert(!parent->relationExists(custom->getType(), custom->getId()));

			custom->initialise(model_file, modelFactory, gameFS, gameDB, scene->textureManager);
			{
				auto mod = std::make_shared<MergedEquipmentGeosetModifier>();
				mod->slot = slot;
				custom->getGeosetTransform().addModifier(mod);
			}
			custom->merge(MergedModel::RESOLUTION_ROUGH);

			Log::message("Loaded merged model / attachment - " + QString::number(custom->getId()));

			//load attachment texture
			auto tex = scene->textureManager.add(texture_file, gameFS);
			if (tex != nullptr) {
				custom->replacableTextures[TextureType::CAPE] = tex;
			}

			auto att = std::make_unique<Attachment>(custom.get(), slot);
			att->attachmentPosition = position;

			
			parent->addRelation(std::move(custom));

			parent->setAttachmentPosition(att.get(), position);

			return std::move(att);
		}
		else {
			return StandardAttachmentCustomizationProvider::makeAttachment(slot, position, wrapper, model_file, texture_file, parent, scene);
		}

		return nullptr; 
	}
	bool MergedAwareAttachmentCustomizationProvider::isMergedType(CharacterSlot slot) const
	{
		return slot != CharacterSlot::HAND_LEFT && slot != CharacterSlot::HAND_RIGHT && slot != CharacterSlot::SHOULDER && slot != CharacterSlot::HEAD && slot != CharacterSlot::BELT;
	}
}