#include "../../stdafx.h"
#include "Model.h"
#include "M2.h"

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

};