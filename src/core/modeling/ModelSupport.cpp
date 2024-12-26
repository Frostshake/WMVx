#include "../../stdafx.h"
#include "ModelSupport.h"

namespace core {

	void ModelTextureInfo::loadTexture(
		const M2Model* model,
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


	void ModelAnimationInfo::initAnimationData(const M2Model* _model) {
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

	void ModelGeosetInfo::initGeosetData(const M2Model* _model, bool default_vis) {
		geosetState.init(_model, default_vis);
	}

}