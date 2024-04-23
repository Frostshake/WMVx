#pragma once

#include <memory>
#include <vector>
#include "../game/GameConstants.h"
#include "RawModel.h"
#include "Animator.h"
#include "ModelSupport.h"
#include "../utility/Vector3.h"
#include "MergedModel.h"

namespace core {
	class Attachment 
	{
	public:

		class Effect : public ModelTextureInfo, public ModelAnimationInfo {
		public:
			Effect(std::unique_ptr<RawModel> raw_model) : 
				model(std::move(raw_model)), itemVisualEffectId(0) {}
			Effect(Effect&&) = default;
			virtual ~Effect() {};

			void update(const Animator& animator, const AnimationTickArgs& tick) {

				model->calculateBones(animator.getAnimationIndex().value(), tick);
				updateAnimation();

				model->updateParticles(animator.getAnimationIndex().value(), tick);
				model->updateRibbons(animator.getAnimationIndex().value(), tick);
			}

			uint32_t itemVisualEffectId;
			std::unique_ptr<RawModel> model;
		};

		Attachment(std::unique_ptr<RawModel> raw_model, CharacterSlot slot);
		Attachment(MergedModel* merged_model, CharacterSlot slot);
		Attachment(Attachment&&) = default;
		virtual ~Attachment() {}

		void update(const Animator& animator, const AnimationTickArgs& tick);

		CharacterSlot getSlot() const;

		void setPosition(AttachmentPosition attach_pos, uint16_t set_bone, const Vector3& set_pos);

		RawModel* getModel() const;

		// model data lives in this attachment.
		struct AttachOwnedModel : public ModelTextureInfo, public ModelAnimationInfo, public ModelGeosetInfo {
		public:
			std::unique_ptr<RawModel> model;
			uint16_t bone;
			Vector3 position;
		};

		// model data lives in the parent model merges (DF+)
		struct AttachMergedModel {
		public:
			MergedModel* model;
			AttachMergedModel() = default;
			AttachMergedModel(AttachMergedModel&) = delete;
			AttachMergedModel(const AttachMergedModel&) = delete;
			AttachMergedModel(AttachMergedModel&&);
			~AttachMergedModel();
		};

		std::variant<AttachOwnedModel, AttachMergedModel> modelData;
		AttachmentPosition attachmentPosition;
		
		template<typename T>
		requires (std::is_same_v<T, AttachOwnedModel> || std::is_same_v<T, AttachMergedModel>)
		void visit(auto callback) {
			T* ptr = std::get_if<T>(&modelData);
			if (ptr != nullptr) {
				callback(ptr);
			}
		}

		template<typename T>
		requires (std::is_same_v<T, AttachOwnedModel> || std::is_same_v<T, AttachMergedModel>)
		void visit(auto callback) const {
			const T* ptr = std::get_if<T>(&modelData);
			if (ptr != nullptr) {
				callback(ptr);
			}
		}

		std::vector<std::unique_ptr<Effect>> effects;

	protected:
		CharacterSlot characterSlot;

	};

}