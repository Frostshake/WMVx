#include "../../stdafx.h"
#include "Model.h"

namespace core {

	Model::Model(ModelFactory& factory) :animator()
	{
		animate = false;
		model = factory();
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

		model->load(fs, uri, loadTexture);

		textureSet.load(fs->asInternal(model->getFileInfo()), db);
		initAnimationData(model.get());
		visibleGeosets.resize(model->getGeosetAdaptors().size(), true);
	}

	void Model::update(uint32_t delta_time_msecs)
	{
		if (animate && animator.getAnimationId().has_value()) {
			const AnimationTickArgs& tick = animator.tick(delta_time_msecs);

			model->calculateBones(animator.getAnimationIndex().value(), tick);
			updateAnimation(model.get());

			model->updateParticles(animator.getAnimationIndex().value(), tick);
			model->updateRibbons(animator.getAnimationIndex().value(), tick);

			for (auto& child : attachments) {
				child->update(animator, tick);
			}
		}
	}

};