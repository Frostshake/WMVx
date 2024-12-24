#include "../../stdafx.h"
#include "../filesystem/GameFileSystem.h"
#include "../database/GameDatabase.h"
#include "MergedModel.h"
#include "Model.h"

namespace core {

	MergedModel::MergedModel(Model* _owner, Type _type, id_t _id) :
		model(nullptr), owner(_owner), type(_type), id(_id), ComponentMeta(ComponentMeta::Type::MERGED)
	{
		assert(owner != nullptr);
	}

	void MergedModel::initialise(const GameFileUri& uri, M2Model::Factory& factory, GameFileSystem* fs, GameDatabase* db, TextureManager& manager)
	{

		{
			auto [m2_ptr, tex_info] = factory(fs, uri);
			model = std::move(m2_ptr);

			for (auto& tex : tex_info) {
				this->loadTexture(model.get(), tex.index, tex.defintion, tex.uri, manager, fs);
			}
		}

		initAnimationData(model.get());
		initGeosetData(model.get(), false);
	}

	void MergedModel::merge(float resolution) {
		// attempt to relate 'our' bones to the owner
		uint16_t bone_index = 0;
		uint16_t owner_bone_index = 0;
		for (const auto* bone : model->getBoneAdaptors()) {
			owner_bone_index = 0;
			for (const auto* owner_bone : owner->model->getBoneAdaptors()) {
				const auto max_pivot_diff = (bone->getPivot() - owner_bone->getPivot())
					.abs()
					.max();
				const bool close_match = max_pivot_diff < resolution;

				if (close_match) {
					boneMap[bone_index] = owner_bone_index;
					break;
				}

				owner_bone_index++;
			}
			bone_index++;
		}
	}

	void MergedModel::update(const Animator& animator, const AnimationTickArgs& tick)
	{
		model->calculateBones(animator.getAnimationIndex().value(), tick);

		//updateAnimation(model.get());
		// use an alternative implementation that can use the owner bones too.
		updateAnimationWithOwner();

		model->updateParticles(animator.getAnimationIndex().value(), tick);
		model->updateRibbons(animator.getAnimationIndex().value(), tick);
	}

	void MergedModel::updateAnimationWithOwner() {
		const auto& boneAdaptors = model->getBoneAdaptors();

		if (boneAdaptors.size() == 0) {
			return;
		}

		auto index = 0;
		for (auto& orgVert : model->getRawVertices()) {
			Vector3 v = Vector3(0, 0, 0);
			Vector3 n = Vector3(0, 0, 0);

			for (size_t b = 0; b < ModelVertexM2::BONE_COUNT; b++)
			{
				if (orgVert.boneWeights[b] > 0) {
					const auto bone_index = orgVert.bones[b];
					const ModelBoneAdaptor* adaptor = boneAdaptors[bone_index];

					if (boneMap.contains(bone_index)) {
						const auto mapped_index = boneMap[bone_index];
						adaptor = owner->model->getBoneAdaptors().at(mapped_index);
					}

					Vector3 tv = adaptor->getMat() * precomputed[index].position;
					Vector3 tn = adaptor->getMRot() * precomputed[index].normal;
					v += tv * ((float)orgVert.boneWeights[b] / 255.0f);
					n += tn * ((float)orgVert.boneWeights[b] / 255.0f);
				}
			}

			animatedVertices[index] = v;
			animatedNormals[index] = n;
			index++;
		}
	}
};