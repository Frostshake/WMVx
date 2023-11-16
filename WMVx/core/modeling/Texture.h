#pragma once
#include <cstdint>
#include "../../OpenGL.h"
#include <QString>
#include "BLP.h"
#include "../filesystem/GameFileSystem.h"
#include <span>
#include "../game/GameConstants.h"
#include "M2Defintions.h"
#include "../database/GameDatasetAdaptors.h"

namespace core {
	typedef GLuint TextureID;

	class Texture {
	public:

		static const GLuint INVALID_ID = 0;

		int32_t width;
		int32_t height;
		GLuint id = INVALID_ID;
		bool compressed;
		GameFileUri fileUri;

		Texture(GameFileUri = 0ul);
		Texture(Texture&&) = default;
		virtual ~Texture() {}

		std::vector<uint8_t> getPixels(uint32_t format = GL_RGBA);
	};

	class TextureManager {
	public:
		TextureManager() = default;
		TextureManager(const TextureManager& instance) = delete;
		TextureManager(TextureManager&&) = default;
		virtual ~TextureManager() {}

		std::shared_ptr<Texture> add(GameFileUri uri, GameFileSystem* fs);

		inline const std::map<TextureID, std::weak_ptr<Texture>>& textures() {
			return textureMap;
		}

	protected:
		void remove(GLuint id);
		void loadBLP(Texture* tex, GameFileSystem* fs);

		std::map<TextureID, std::weak_ptr<Texture>> textureMap;
	};


	class CharacterTextureBuilder {
	public:

		void setBaseLayer(const GameFileUri& textureUri);
		void pushBaseLayer(const GameFileUri& textureUri);
		void addLayer(const GameFileUri& textureUri, CharacterRegion region, int layer_index, BlendMode blend_mode = BlendMode::BM_OPAQUE);

		std::shared_ptr<Texture> build(CharacterComponentTextureAdaptor* componentTextureAdaptor, TextureManager* manager, GameFileSystem* fs);

	private:

		void mergeLayer(const GameFileUri& uri, TextureManager* manager, GameFileSystem* fs, std::vector<uint8_t>& dest, int32_t dest_width, CharacterRegionCoords coords);

		struct Component {
			GameFileUri uri = 0ul;
			CharacterRegion region = (CharacterRegion)0;
			int layerIndex = 0;
			BlendMode blendMode = BlendMode::BM_OPAQUE;

			const bool operator<(const Component& c) const
			{
				return layerIndex < c.layerIndex;
			}
		};

		std::vector<Component> components;
		std::vector<GameFileUri> baseLayers;
	};
}