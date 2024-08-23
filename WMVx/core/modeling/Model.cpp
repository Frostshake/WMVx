#include "../../stdafx.h"
#include "Model.h"
#include "M2.h"

namespace core {

	Model::Model(RawModel::Factory& factory) :
		ComponentMeta(ComponentMeta::Type::ROOT),
		animator()
	{
		animate = false;
		model = factory();
		characterInitialised = false;
	}

	void Model::initialise(const GameFileUri& uri, GameFileSystem* fs, GameDatabase* db, TextureManager& manager)
	{
		auto loadTexture = std::bind(&ModelTextureInfo::loadTexture,
			this,
			std::placeholders::_1,
			std::placeholders::_2,
			std::placeholders::_3,
			std::placeholders::_4,
			std::ref(manager),
			fs
		);

		{
			M2Model Model(fs, uri);
			int a = 5;
			a++;
		}

		model->load(fs, uri, loadTexture);

		textureSet.load(fs->asInternal(model->getFileInfo()), db);
		initAnimationData(model.get());
		initGeosetData(model.get());

		characterDetails = ([&]() -> std::optional<CharacterDetails> {
			
			if (model->isCharacter())
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