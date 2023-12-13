#pragma once
#include "RawModel.h"
#include "Animator.h"
#include "Attachment.h"
#include "MergedModel.h"
#include "ModelSupport.h"
#include <memory>
#include <vector>
#include "../game/GameClientAdaptor.h"
#include "TabardCustomization.h"
#include "TextureSet.h"

namespace core {

	/// <summary>
	/// Helper class for managing an equiped item, specifically for when a 'item' record doesnt exist in the DB, so a dummy is created which is owned by wrapper.
	/// </summary>
	class CharacterItemWrapper {
	public:
		CharacterItemWrapper() : item(nullptr), display(nullptr), owning(false) {}
		CharacterItemWrapper(CharacterItemWrapper&& other) {
			owning = other.owning;
			other.owning = false;
			item = other.item;
			display = other.display;
		}
		CharacterItemWrapper& operator=(CharacterItemWrapper&& other) {
			owning = other.owning;
			other.owning = false;
			item = other.item;
			display = other.display;
			return *this;
		}
		virtual ~CharacterItemWrapper() {
			if (owning) {
				// display will never be owned, only the dummy item record;
				if (item != nullptr) {
					delete item;
				}
			}
		}

		static CharacterItemWrapper make(const ItemRecordAdaptor* item, GameDatabase* db) {
			const auto* display = db->itemDisplayDB->findById(item->getItemDisplayInfoId());
			if (display == nullptr) {
				throw std::runtime_error("Unable to find display record.");
			}
			return CharacterItemWrapper(item, display, false);
		}

		static CharacterItemWrapper make(ItemInventorySlotId slot, const ItemDisplayRecordAdaptor* display) {
			auto* dummy = new DummyItemRecordAdaptor();
			dummy->display_info_id = display->getId();
			dummy->inventory_slot_id = slot;
			dummy->sheath_type = SheathTypes::SHEATHETYPE_NONE;
			dummy->name = QString("Display Id: %1").arg(dummy->display_info_id);
			return CharacterItemWrapper(dummy, display, true);
		}

		const ItemRecordAdaptor* item;
		const ItemDisplayRecordAdaptor* display;
	protected:

		class DummyItemRecordAdaptor : public ItemRecordAdaptor {
		public:
			constexpr uint32_t getId() const override {
				return 0;
			}

			constexpr uint32_t getItemDisplayInfoId() const override {
				return display_info_id;
			}

			constexpr ItemInventorySlotId getInventorySlotId() const override {
				return inventory_slot_id;
			}

			constexpr SheathTypes getSheatheTypeId() const override {
				return sheath_type;
			}

			constexpr ItemQualityId getItemQuality() const override {
				return ItemQualityId::NORMAL;
			}

			QString getName() const override {
				return name;
			}

			uint32_t display_info_id;
			ItemInventorySlotId inventory_slot_id;
			SheathTypes sheath_type;
			QString name;
		};

		CharacterItemWrapper(const ItemRecordAdaptor* item_adaptor,
			const ItemDisplayRecordAdaptor* display_adaptor,
			bool is_owning) :
			item(item_adaptor),
			display(display_adaptor),
			owning(is_owning) {}

		CharacterItemWrapper(const CharacterItemWrapper&) = default;
		CharacterItemWrapper& operator=(CharacterItemWrapper const&) = default;

		bool owning;
	};

	class Model : public ModelTextureInfo, public ModelAnimationInfo, public ModelGeosetInfo
	{
	public:
		Model(ModelFactory& factory);
		Model(Model&&) = default;
		virtual ~Model() {}

		void initialise(const GameFileUri& uri, GameFileSystem* fs, GameDatabase* db, TextureManager& manager);

		void update(uint32_t delta_time_msecs);

		std::unique_ptr<RawModel> model;
		TextureSet textureSet;	

		// character specific options
		std::map<CharacterSlot, CharacterItemWrapper> characterEquipment;
		CharacterCustomization characterCustomizationChoices;
		std::optional<TabardCustomization> tabardCustomization;
		CharacterRenderOptions characterOptions;
		//


		bool animate;
		Animator animator;

		ModelRenderOptions renderOptions;

		const std::vector<Attachment*>& getAttachments() const {
			return reinterpret_cast<const std::vector<Attachment*>&>(attachments);
		}

		void addAttachment(std::unique_ptr<Attachment> attachment) {
			
			auto pos = attachment->attachmentPosition;
			std::erase_if(attachments, [pos](const std::unique_ptr<Attachment>& att) -> bool {
				return att->attachmentPosition == pos;
			});

			attachments.push_back(std::move(attachment));
		}

		void removeAttachments(CharacterSlot slot) {
			std::erase_if(attachments, [slot](const std::unique_ptr<Attachment>& att) -> bool {
				return att->characterSlot == slot;
			});
		}

		void setAttachmentPosition(Attachment* attachment, AttachmentPosition position) {

			if (position >= AttachmentPosition::MAX) {
				throw std::runtime_error("Attachment position not valid.");
			}

			attachment->attachmentPosition = position;

			auto lookup_val = model->getAttachmentLookups()[(size_t)position];
			const auto& attachDef = model->getAttachmentDefintionAdaptors().at(lookup_val);
			attachment->bone = attachDef->getBone();
			attachment->position = Vector3::yUpToZUp(attachDef->getPosition());
		}

		const std::vector<MergedModel*>& getMerged() const {
			return reinterpret_cast<const std::vector<MergedModel*>&>(merged);
		}

		void addRelation(std::unique_ptr<MergedModel> relation) {
			merged.push_back(std::move(relation));
		}

		void removeRelation(MergedModel::Type type, MergedModel::id_t id) {
			std::erase_if(merged, [type, id](const std::unique_ptr<MergedModel>& rel) -> bool {
				return rel->getType() == type && rel->getId() == id;
			});
		}

	protected:

		std::vector<std::unique_ptr<Attachment>> attachments;
		std::vector<std::unique_ptr<MergedModel>> merged;

	};

};