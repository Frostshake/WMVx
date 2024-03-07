#include "../../stdafx.h"
#include "ModelSupport.h"

namespace core {

	void ModelTextureInfo::loadTexture(
		const RawModel* model,
		size_t index,
		const ModelTextureM2& textureDefinition,
		GameFileUri uri,
		TextureManager& textureManager,
		GameFileSystem* gameFS) {

		if (textureDefinition.type == (uint32_t)TextureType::FILENAME) {
			assert(!uri.isEmpty());
			Log::message("loadTextures: " + uri.toString());

			auto texture = textureManager.add(uri, gameFS);
			if (texture != nullptr) {
				textures[index] = texture;
			}
		}
		else {
			specialTextures[index] = (TextureType)textureDefinition.type;
		}
	}


	void ModelAnimationInfo::initAnimationData(const RawModel* _model) {
		model = _model;
		animatedVertices.clear();
		animatedNormals.clear();
		precomputed.clear();

		animatedVertices = model->getVertices();
		animatedNormals = model->getNormals();

		const auto& orgVerts = model->getRawVertices();
		precomputed.reserve(orgVerts.size());

		for (const auto& orgVert : orgVerts) {
			precomputed.emplace_back(
				Vector3::yUpToZUp(orgVert.position),
				Vector3::yUpToZUp(orgVert.normal).normalize()
			);
		}
	}

	void ModelAnimationInfo::updateAnimation() {

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
					const auto& adaptor = boneAdaptors[orgVert.bones[b]];
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

	void ModelGeosetInfo::initGeosetData(const RawModel* _model, bool default_vis) {
		model = _model;
		visibleGeosets.resize(model->getGeosetAdaptors().size(), default_vis);
	}

	void ModelGeosetInfo::forceGeosetVisibilityById(uint32_t id, bool visible) {
		if (model != nullptr) {
			size_t index = 0;
			const auto& adaptors = model->getGeosetAdaptors();
			for (const auto& adaptor : adaptors) {
				const auto adaptor_id = adaptor->getId();
				if (adaptor_id == id) {
					visibleGeosets[index] = visible;
				}

				index++;
			}
		}
	}

	void ModelGeosetInfo::setGeosetVisibility(CharacterGeosets geoset, uint32_t flags)
	{
		//formula for converting a geoset flag into an id
		//xx1 id's look to be the default, so +1 gets added to the flags
		const auto geoset_id = (geoset * 100) + 1 + flags;

		const auto id_range_start = geoset * 100;
		const auto id_range_end = (geoset + 1) * 100;

		assert(id_range_start <= geoset_id);
		assert(id_range_end >= geoset_id);

		if (model != nullptr) {
			size_t index = 0;
			const auto& adaptors = model->getGeosetAdaptors();
			for (const auto& adaptor : adaptors) {
				const auto adaptor_id = adaptor->getId();
				if (adaptor_id == geoset_id) {
					visibleGeosets[index] = true;
				}
				else if (id_range_start < adaptor_id && adaptor_id < id_range_end) {
					visibleGeosets[index] = false;
				}

				index++;
			}
		}
	}

	void ModelGeosetInfo::clearGeosetVisibility(CharacterGeosets geoset) {
		const auto id_range_start = geoset * 100;
		const auto id_range_end = (geoset + 1) * 100;

		if (model != nullptr) {
			size_t index = 0;
			const auto& adaptors = model->getGeosetAdaptors();
			for (const auto& adaptor : adaptors) {
				const auto adaptor_id = adaptor->getId();
				if (id_range_start < adaptor_id && adaptor_id < id_range_end) {
					visibleGeosets[index] = false;
				}

				index++;
			}
		}
	}

}