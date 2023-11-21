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
	class Model : public ModelTextureInfo, public ModelAnimationInfo
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
		std::map<CharacterSlot, const ItemRecordAdaptor*> characterEquipment;
		CharacterCustomization characterCustomizationChoices;
		std::optional<TabardCustomization> tabardCustomization;
		CharacterRenderOptions characterOptions;
		//


		bool animate;
		Animator animator;

		std::vector<bool> visibleGeosets;	// vector index corrisponds to getGeosets index.
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