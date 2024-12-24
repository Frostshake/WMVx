#include "../../stdafx.h"
#include "Model.h"
#include "M2.h"
#include "Scene.h"

namespace core {

	Model::Model() :
		ComponentMeta(ComponentMeta::Type::ROOT),
		animator()
	{
		animate = false;
		model = nullptr;
		characterInitialised = false;
	}

	void Model::initialise(const GameFileUri& uri, M2Model::Factory& factory, GameFileSystem* fs, GameDatabase* db, TextureManager& manager)
	{

		{
			auto [m2_ptr, tex_info] = factory(fs, uri);
			model = std::move(m2_ptr);

			for (auto& tex : tex_info) {
				this->loadTexture(model.get(), tex.index, tex.defintion, tex.uri, manager, fs);
			}
		}

		textureSet.load(fs->asInternal(model->getFileInfo()), db);
		initAnimationData(model.get());
		initGeosetData(model.get());

		characterDetails = ([&]() -> std::optional<CharacterDetails> {
			
			if (model->getModelPathInfo().isCharacter())
			{
				CharacterDetails temp;
				const bool found = CharacterDetails::detect(this, db, temp);
				assert(found);
				if (found)
				{
					return temp;
				}
			}

			return std::nullopt;
		})();

	}

	void Model::update(uint32_t delta_time_msecs)
	{
		if (animate && animator.getAnimationId().has_value()) {
			const AnimationTickArgs& tick = animator.tick(delta_time_msecs);

			model->calculateBones(animator.getAnimationIndex().value(), tick);
			updateAnimation();

			model->updateParticles(animator.getAnimationIndex().value(), tick);
			model->updateRibbons(animator.getAnimationIndex().value(), tick);

			for (auto& child : attachments) {
				child->update(animator, tick);
			}

			for (auto& rel : merged) {
				rel->update(animator, tick);
			}
		}
	}

	void ModelHelper::addItem(CharacterSlot slot, const core::CharacterItemWrapper& wrapper, std::function<void(Attachment*, uint32_t)> visual_handler) {
		assert(_attach_provider != nullptr);
		//TODO only update if needed - currenlty item gets removed and re-added even if no changes are needed.
		_model->removeAttachments(slot);

		const auto attach_positions = _attach_provider->getAttachmentPositions(slot, wrapper.item(), _model->characterOptions.sheatheWeapons);

		const auto* item_display = wrapper.display();
		const auto& char_details = _model->getCharacterDetails();

		auto attachment_index = 0;
		const auto item_models = item_display->getModel(slot, wrapper.item()->getInventorySlotId(), *_model_context);
		const auto item_textures = item_display->getModelTexture(slot, wrapper.item()->getInventorySlotId(), *_texture_context);
		assert(item_models.size() >= attach_positions.size());
		assert(item_textures.size() >= attach_positions.size());

		for (auto attach_pos : attach_positions) {

			GameFileUri model_path = item_models[attachment_index];
			GameFileUri texture_path = item_textures[attachment_index];

			if (model_path.isEmpty()) {
				continue;
			}

			if (model_path.isPath()) {
				model_path = GameFileUri::replaceExtension(model_path.getPath(), "m2");
			}

			try {

				Log::message("Loaded attachment model: " + model_path.toString());
				Log::message("Loaded attachment texture: " + texture_path.toString());

				std::unique_ptr<Attachment> att = _attach_provider->makeAttachment(
					slot,
					attach_pos,
					wrapper,
					model_path,
					texture_path,
					_model,
					_scene
				);

				//TODO move into attachment provider.
				//TODO handle item visuals for BFA+
				const auto itemVisualId = item_display->getItemVisualId();

				if (itemVisualId > 0 && visual_handler) {
					visual_handler(att.get(), itemVisualId);
				}

				QString display_name = wrapper.item()->getName();
				if (display_name.length() > 0) {
					att->setMetaName(
						QString("%1 [%2]")
						.arg(display_name)
						.arg(item_display->getId())
					);
				}


				{
					auto* tmp = att.get();
					_model->addAttachment(std::move(att));
					_scene->addComponent(tmp);
				}

			}
			catch (std::exception& e) {
				Log::message(QString("Exception caught loading attachment %1:").arg(attachment_index));
				Log::message(e.what());
				throw;
			}

			attachment_index++;
		}
	}

	void ModelHelper::removeItem(CharacterSlot slot) {
		_model->removeAttachments(slot);
	}

};