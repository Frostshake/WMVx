#include "../../stdafx.h"
#include "Attachment.h"
#include "Model.h"

namespace core {

	Attachment::Attachment(std::unique_ptr<RawModel> raw_model, CharacterSlot slot):
		ComponentMeta(ComponentMeta::Type::ATTACHMENT)
	{
		AttachOwnedModel owned;
		owned.model = std::move(raw_model);
		owned.bone = 0;
		owned.position = Vector3();
		modelData.emplace<AttachOwnedModel>(std::move(owned));
		characterSlot = slot;
	}

	Attachment::Attachment(MergedModel* merged_model, CharacterSlot slot) :
		ComponentMeta(ComponentMeta::Type::ATTACHMENT)
	{
		AttachMergedModel merged;
		merged.model = merged_model;
		modelData.emplace<AttachMergedModel>(std::move(merged));
		characterSlot = slot;
	}

	void Attachment::update(const Animator& animator, const AnimationTickArgs& tick) {

		visit<AttachOwnedModel>([&](AttachOwnedModel* owned) {
			owned->model->calculateBones(animator.getAnimationIndex().value(), tick);
			owned->updateAnimation();

			owned->model->updateParticles(animator.getAnimationIndex().value(), tick);
			owned->model->updateRibbons(animator.getAnimationIndex().value(), tick);

		});
		
		for (auto& effect : effects) {
			effect->update(animator, tick);
		}
	}

	CharacterSlot Attachment::getSlot() const {
		return characterSlot;
	}

	void Attachment::setPosition(AttachmentPosition attach_pos, uint16_t set_bone, const Vector3& set_pos) {
		attachmentPosition = attach_pos;
		visit<AttachOwnedModel>([&](AttachOwnedModel* owned) {
			owned->bone = set_bone;
			owned->position = set_pos;
		});
	}

	RawModel* Attachment::getModel() const
	{
		return std::visit([](const auto& data) -> RawModel* {
			if constexpr (std::is_same_v<const Attachment::AttachOwnedModel&, decltype(data)>) {
				return data.model.get();
			}
			else if constexpr (std::is_same_v<const Attachment::AttachMergedModel&, decltype(data)>) {
				return data.model->model.get();
			}

			return nullptr;
		}, modelData);
	}

	Attachment::AttachMergedModel::AttachMergedModel(AttachMergedModel&& source)
	{
		model = source.model;
		source.model = nullptr;
	}

	Attachment::AttachMergedModel::~AttachMergedModel() {
		if (model != nullptr && model->owner != nullptr) {
			model->owner->removeRelation(model->getType(), model->getId());
			model = nullptr;
		}
	}
}