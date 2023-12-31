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

		enum EyeGlow : uint8_t {
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

		ModelRenderOptions(const ModelRenderOptions&) = default;
		ModelRenderOptions(ModelRenderOptions&&) = default;
	};

	class ModelTextureInfo {
	public:
		ModelTextureInfo() = default;
		ModelTextureInfo(ModelTextureInfo&&) = default;
		virtual ~ModelTextureInfo() {}

		std::map<size_t, std::shared_ptr<Texture>> textures;
		std::map<size_t, TextureType> specialTextures;	
		std::map<TextureType, std::shared_ptr<Texture>> replacableTextures; 

		void loadTexture(const RawModel* model,
			size_t index,
			const ModelTextureM2& textureDefinition,
			GameFileUri uri,
			TextureManager& textureManager,
			GameFileSystem* gameFS);
		
		inline GLuint getTextureId(const int32_t render_pass_tex) const {
			if (specialTextures.contains(render_pass_tex)) {
				const auto special = specialTextures.at(render_pass_tex);
				if (replacableTextures.contains(special)) {
					return replacableTextures.at(special)->id;
				}
			}
			else if (textures.contains(render_pass_tex)) {
				return textures.at(render_pass_tex)->id;
			}

			return Texture::INVALID_ID;
		}

	};

	class ModelAnimationInfo {
	public:
		ModelAnimationInfo() = default;
		ModelAnimationInfo(ModelAnimationInfo&&) = default;
		virtual ~ModelAnimationInfo() {}

		std::vector<Vector3> animatedVertices;
		std::vector<Vector3> animatedNormals;

		void initAnimationData(const RawModel* model);

		void updateAnimation();

	protected:
		struct VertData {
			Vector3 position;
			Vector3 normal;
		};

		//purely for speed, we convert the data from raw format and store for use.
		std::vector<VertData> precomputed;
	private:
		const RawModel* model;
	};

	class ModelGeosetInfo {
	public:
		ModelGeosetInfo() = default;
		ModelGeosetInfo(ModelGeosetInfo&&) = default;
		virtual ~ModelGeosetInfo() {}

		void initGeosetData(const RawModel* _model, bool default_vis = true);

		inline void forceGeosetVisibilityByIndex(size_t index, bool visible) {
			visibleGeosets[index] = visible;
		}

		void forceGeosetVisibilityById(uint32_t id, bool visible);

		inline bool isGeosetIndexVisible(size_t index) const {
			return visibleGeosets[index];
		}

		void setGeosetVisibility(CharacterGeosets geoset, uint32_t flags);
		void clearGeosetVisibility(CharacterGeosets geoset);		
		
	private:
		std::vector<bool> visibleGeosets;	// vector index corrisponds to getGeosets index.

		const RawModel* model;
	};
};