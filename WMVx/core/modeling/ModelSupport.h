#pragma once

#include <optional>
#include <map>
#include <vector>
#include "../utility/Vector3.h"
#include "../game/GameConstants.h"
#include "../database/WOTLKRecordDefinitions.h"
#include "../filesystem/GameFileSystem.h"
#include "Texture.h"
#include "../utility/Logger.h"
#include "../database/GameDatasetAdaptors.h"
#include "../modeling/RawModel.h"

namespace core {

	struct CharacterRenderOptions {

		enum EyeGlow {
			NONE,
			NORMAL,
			DEATH_KNIGHT
		};

		EyeGlow eyeGlow = EyeGlow::NORMAL;
		bool showUnderWear = true;
		bool showEars = true;
		bool showFeet = true;
		bool showHair = true;
		bool showFacialHair = true;
		bool sheatheWeapons = true;
	};

	struct ModelRenderOptions {
		bool showWireFrame;
		bool showBounds;
		bool showBones;
		bool showTexture;
		bool showRender;
		bool showParticles;

		Vector3 position;
		Vector3 rotation;
		Vector3 scale;
		float opacity;

		ModelRenderOptions() : scale(1.f, 1.f, 1.f) {
			showWireFrame = false;
			showBounds = false;
			showBones = false;
			showTexture = true;
			showRender = true;
			showParticles = true;
			opacity = 1.f;
		}
	};

	class ModelTextureInfo {
	public:
		ModelTextureInfo() = default;
		virtual ~ModelTextureInfo() {}

		std::map<size_t, std::shared_ptr<Texture>> textures;
		std::map<size_t, TextureType> specialTextures;	
		std::map<TextureType, std::shared_ptr<Texture>> replacableTextures; 

		void loadTexture(const RawModel* model, 
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
	};

	class ModelAnimationInfo {
	public:
		ModelAnimationInfo() = default;
		virtual ~ModelAnimationInfo() {}

		std::vector<Vector3> animatedVertices;
		std::vector<Vector3> animatedNormals;

		void initAnimationData(const RawModel* model) {
			animatedVertices.clear();
			animatedNormals.clear();

			animatedVertices = model->getVertices();
			animatedNormals = model->getNormals();
		}

		void updateAnimation(const RawModel* model) {
			auto index = 0;
			for (auto& orgVert : model->getRawVertices()) {
				Vector3 v = Vector3(0, 0, 0);
				Vector3 n = Vector3(0, 0, 0);

				for (size_t b = 0; b < ModelVertexM2::BONE_COUNT; b++)
				{
					if (orgVert.boneWeights[b] > 0) {
						const auto& adaptor = model->getBoneAdaptors()[orgVert.bones[b]];
						Vector3 tv = adaptor->getMat() * Vector3::yUpToZUp(orgVert.position);
						Vector3 tn = adaptor->getMRot() * Vector3::yUpToZUp(orgVert.normal).normalize();
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
};