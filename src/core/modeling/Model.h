#pragma once
#include "M2.h"
#include "Animator.h"
#include "Attachment.h"
#include "MergedModel.h"
#include "ModelSupport.h"
#include <memory>
#include <vector>
#include "../game/GameClientAdaptor.h"
#include "TabardCustomization.h"
#include "TextureSet.h"
#include "ComponentMeta.h"


namespace core {

	/// <summary>
	/// Helper class for managing an equiped item, specifically for when a 'item' record doesnt exist in the DB, so a dummy is created which is owned by wrapper.
	/// </summary>
	class CharacterItemWrapper {
	public:
		CharacterItemWrapper() : _item(nullptr), _display(nullptr), _owning(false), _owned_item(nullptr) {}
		virtual ~CharacterItemWrapper() {};

		static CharacterItemWrapper make(const ItemRecordAdaptor* item, GameDatabase* db, uint32_t display_id) {
			const auto display_ids = item->getItemDisplayInfoId();
			auto found = std::find(display_ids.begin(), display_ids.end(), display_id);
			if (found == display_ids.end()) {
				throw std::runtime_error("Invalid display id.");
			}

			const auto* display = db->itemDisplayDB->findById(display_id);
			if (display == nullptr) {
				throw std::runtime_error("Unable to find display record.");
			}
			return CharacterItemWrapper(item, display);
		}

		static CharacterItemWrapper make(ItemInventorySlotId slot, const ItemDisplayRecordAdaptor* display) {
			std::shared_ptr<ItemRecordAdaptor> dummy_src = std::make_shared<DummyItemRecordAdaptor>();
			auto* dummy = (DummyItemRecordAdaptor*)dummy_src.get();
			dummy->display_info_id = display->getId();
			dummy->inventory_slot_id = slot;
			dummy->sheath_type = SheathTypes::SHEATHETYPE_NONE;
			dummy->name = QString("Display Id: %1").arg(dummy->display_info_id);
			return CharacterItemWrapper(dummy_src, display);
		}

		const ItemRecordAdaptor* item() const {

			if (_owning) {
				return _owned_item.get();
			}

			return _item;
		}

		const ItemDisplayRecordAdaptor* display() const {
			return _display;
		}

		bool operator==(const CharacterItemWrapper& rhs) const {
			return (item()->getId() == rhs.item()->getId()) && 
				(display()->getId() && rhs.display()->getId());
		}

	protected:

		const ItemRecordAdaptor* _item;
		std::shared_ptr<ItemRecordAdaptor> _owned_item;
		const ItemDisplayRecordAdaptor* _display;

		class DummyItemRecordAdaptor : public ItemRecordAdaptor {
		public:
			constexpr uint32_t getId() const override {
				return 0;
			}

			constexpr std::vector<uint32_t> getItemDisplayInfoId() const override {
				return { display_info_id };
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
			const ItemDisplayRecordAdaptor* display_adaptor) :
			_item(item_adaptor),
			_display(display_adaptor),
			_owning(false),
			_owned_item(nullptr) {}

		CharacterItemWrapper(std::shared_ptr<ItemRecordAdaptor> owned_item_adaptor,
			const ItemDisplayRecordAdaptor* display_adaptor) :
		_item(nullptr),
		_display(display_adaptor),
		_owning(true),
		_owned_item(owned_item_adaptor)
		{}

		bool _owning;
	};


	class Model : public ModelTextureInfo,
		public ModelAnimationInfo, 
		public ModelGeosetInfo, 
		public HasMergedModels, 
		public ComponentMeta
	{
	public:
		Model();
		Model(Model&&) = default;
		virtual ~Model() {}

		void initialise(const GameFileUri& uri, M2Model::Factory& factory, GameFileSystem* fs, GameDatabase* db, TextureManager& manager);

		void update(uint32_t delta_time_msecs);


		

		std::unique_ptr<M2Model> model;
		TextureSet textureSet;

		// character specific options
		std::map<CharacterSlot, CharacterItemWrapper> characterEquipment;
		CharacterCustomizations characterCustomizationChoices;
		std::optional<TabardCustomizationOptions> tabardCustomizationChoices;
		CharacterRenderOptions characterOptions;
		bool characterInitialised;
		//

		bool animate;
		Animator animator;
		ModelRenderOptions modelOptions;

		virtual GameFileInfo getMetaGameFileInfo() const override {
			return model->getFileInfo();
		}

		virtual std::vector<ComponentMeta*> getMetaChildren() const override {
			std::vector<ComponentMeta*> result;
			result.reserve(attachments.size());

			for (const auto& att : attachments) {
				// merged models get handled seperately.
				att->visit<Attachment::AttachOwnedModel>([&](Attachment::AttachOwnedModel* owned) {
					result.push_back(att.get());
				});
			}

			for (const auto& merge : merged) {
				result.push_back(merge.get());
			}

			return result;
		}

		const std::vector<Attachment*>& getAttachments() const {
			return reinterpret_cast<const std::vector<Attachment*>&>(attachments);
		}

		void addAttachment(std::unique_ptr<Attachment> attachment) {
			const auto pos = attachment->attachmentPosition;
			const auto slot = attachment->getSlot();

			std::erase_if(attachments, [pos, slot](const std::unique_ptr<Attachment>& att) -> bool {
				return att->attachmentPosition == pos && att->getSlot() == slot;
			});

			attachments.push_back(std::move(attachment));
		}


		void removeAttachments(CharacterSlot slot) {
			std::erase_if(attachments, [&, slot](const std::unique_ptr<Attachment>& att) -> bool {
				return att->getSlot() == slot;
			});
		}

		void setAttachmentPosition(Attachment* attachment, AttachmentPosition position) const {

			if (position >= AttachmentPosition::MAX) {
				throw std::runtime_error("Attachment position not valid.");
			}

			auto lookup_val = model->getAttachmentLookups()[(size_t)position];
			const auto& attachDef = model->getAttachmentDefintionAdaptors().at(lookup_val);
			
			attachment->setPosition(
				position, 
				attachDef->getBone(), 
				Vector3::yUpToZUp(attachDef->getPosition())
			);
		}

		const std::optional<CharacterDetails>& getCharacterDetails() const {
			assert(model->getModelPathInfo().isCharacter() == characterDetails.has_value());
			return characterDetails;
		}

	protected:
			
		std::vector<std::unique_ptr<Attachment>> attachments;
		std::optional<CharacterDetails> characterDetails;

	};

	class Scene;

	class ModelHelper {
	public:
		ModelHelper(Scene* scene, Model* model) :
			_scene(scene), _model(model), _attach_provider(nullptr), _model_context(nullptr), _texture_context(nullptr)
		{
		}

		ModelHelper& with(AttachmentCustomizationProvider* provider) {
			_attach_provider = provider;
			return *this;
		}

		ModelHelper& with(std::optional<core::CharacterRelationSearchContext>* model_ctx, std::optional<core::CharacterRelationSearchContext>* tex_ctx) {
			_model_context = model_ctx;
			_texture_context = tex_ctx;
			return *this;
		}

		void addItem(CharacterSlot slot, const core::CharacterItemWrapper& wrapper, std::function<void(Attachment*, uint32_t)> visual_handler);
		void removeItem(CharacterSlot slot);

	protected:
		Scene* _scene;
		Model* _model;
		AttachmentCustomizationProvider* _attach_provider;
		std::optional<core::CharacterRelationSearchContext>* _model_context;
		std::optional<core::CharacterRelationSearchContext>* _texture_context;
	};

};