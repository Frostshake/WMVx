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

	class ArchiveFile;

	typedef GLuint TextureID;

	class BLPLoader {
	public:
		using callback_t = std::function<void(int32_t, uint32_t, uint32_t, void*)>;

		BLPLoader(ArchiveFile* file);
		const BLPHeader& getHeader() const;
		void loadAll(callback_t fn);
		void loadFirst(callback_t fn);
	
	private:
		void load(int32_t mip_count, callback_t fn);

		ArchiveFile* source;
		BLPHeader header;
		std::vector<uint8_t> buffer;
	};

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

		enum class BlendMode : uint32_t {
			NONE = 0,
			BLIT = 1,
			MULTIPLY = 4,
			OVERLAY = 6,
			SCREEN = 7,
			ALPHA_STRAIGHT = 9,
			INFER_ALPHA_BLEND = 15,

		};

		void setBaseLayer(const GameFileUri& textureUri);
		void pushBaseLayer(const GameFileUri& textureUri);
		void addLayer(const GameFileUri& textureUri, CharacterRegion region, int layer_index, BlendMode blend_mode = BlendMode::BLIT);

		std::shared_ptr<Texture> build(CharacterComponentTextureAdaptor* componentTextureAdaptor, TextureManager* manager, GameFileSystem* fs);

	private:

		struct TextureBufferInfo {
			const uint8_t* data;
			const int32_t width;
			const int32_t height;
		};

		void mergeLayer(const GameFileUri& uri, TextureManager* manager, GameFileSystem* fs, const TextureBufferInfo& buffer_info, const CharacterRegionCoords& coords, BlendMode blendMode);

		struct Component {
			GameFileUri uri = 0ul;
			CharacterRegion region = (CharacterRegion)0;
			int layerIndex = 0;
			BlendMode blendMode = BlendMode::BLIT;

			const bool operator<(const Component& c) const
			{
				return layerIndex < c.layerIndex;
			}
		};

		std::vector<Component> components;
		std::vector<GameFileUri> baseLayers;
	};
}