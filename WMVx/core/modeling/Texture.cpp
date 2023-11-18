#include "../../stdafx.h"
#include "Texture.h"
#include <bitset>
#include "../utility/Logger.h"
#include "../utility/Exceptions.h"
#include "../utility/ScopeGuard.h"

#include <ximage.h> 
#include "../../ddslib.h"

#include <QImage>
#include <QPoint>

namespace core {

	BLPLoader::BLPLoader(ArchiveFile* file) :source(file) {
		auto size = file->getFileSize();
		if (size < sizeof(header)) {
			throw FileIOException("File is smaller than BLP header.");
		}
		file->read(&header, sizeof(header));

		std::string signature((char*)header.signature, sizeof(header.signature));
		if (signature != "BLP2") {
			//TODO throw real file name
			throw BadSignatureException("BLP File", signature, "BLP2");
		}
	}

	const BLPHeader& BLPLoader::getHeader() const {
		return header;
	}

	void BLPLoader::load(int32_t mip_count, callback_t fn) {
		bool video_support_compression = false; //TODO detect / config

		auto bufferSize = source->getFileSize();
		auto buffer = std::vector<uint8_t>(bufferSize);
		source->read(buffer.data(), bufferSize);

		uint32_t w = header.width;
		uint32_t h = header.height;

		switch (header.colorEncoding) {
		case BLPColorEncoding::COLOR_PALETTE:
		{
			std::span<uint32_t> pal_view((uint32_t*)(buffer.data() + sizeof(BLPHeader)), sizeof(uint32_t) * 256);

			bool has_alpha = header.alphaSize != 0;

			//TODO CHECK LOGIC
			for (auto i = 0; i < mip_count; i++) {
				if (w == 0) w = 1;
				if (h == 0) h = 1;

				auto out_buffer = std::vector<uint32_t>(header.width * header.height);

				if (header.mipOffsets[i] && header.mipSizes[i]) {
					std::span<uint8_t> buffer_view((uint8_t*)(buffer.data() + header.mipOffsets[i]), header.mipSizes[i]);

					int alpha = 0;
					auto buffer_index = 0;

					std::span<uint8_t> alpha_view;
					auto alpha_index = 0;
					if (header.alphaSize > 0) {
						alpha_view = std::span((uint8_t*)(buffer_view.data() + (w * h)), header.mipSizes[i] - (w * h));
					}

					uint8_t alpha_sub_offset = 0;

					for (uint32_t y = 0; y < h; y++) {
						for (uint32_t x = 0; x < w; x++) {

							uint32_t k = pal_view[buffer_view[buffer_index]];

							k = ((k & 0x00FF0000) >> 16) | ((k & 0x0000FF00)) | ((k & 0x000000FF) << 16);

							if (has_alpha) {
								if (header.alphaSize == 8) {
									alpha = alpha_view[alpha_index++];
								}
								else if (header.alphaSize == 4) {
									alpha = (alpha_view[alpha_index] & (0xf << alpha_sub_offset++)) * 0x11;
									if (alpha_sub_offset == 2)
									{
										alpha_sub_offset = 0;
										alpha_index++;
									}
								}
								else if (header.alphaSize == 1) {
									alpha = (alpha_view[alpha_index] & (1 << alpha_sub_offset++)) ? 0xff : 0;
									if (alpha_sub_offset == 8)
									{
										alpha_sub_offset = 0;
										alpha_index++;
									}
								}
							}
							else {
								alpha = 0xff;
							}

							k |= alpha << 24;
							out_buffer[buffer_index] = k;
							buffer_index++;

						}
					}

					fn(i, w, h, out_buffer.data());
				}
				else {
					break;
				}

				w >>= 1;
				h >>= 1;
			}
		}
		break;
		case BLPColorEncoding::COLOR_DXT:
		{
			GLint format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			int32_t blockSize = 8;

			//TODO CHECK LOGIC!

			if (header.alphaSize == 8 || header.alphaSize == 4) {
				format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
				blockSize = 16;
			}

			if (header.alphaSize == 8 && header.preferredFormat == 7) {
				format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
				blockSize = 16;
			}

			auto uncompressed_buffer = std::vector<uint8_t>();

			if (!video_support_compression) {
				uncompressed_buffer.resize(header.width * header.height * 4);
			}

			for (auto i = 0; i < mip_count; i++) {
				if (w == 0) w = 1;
				if (h == 0) h = 1;

				if (header.mipOffsets[i] && header.mipSizes[i]) {

					//mips offset already include the header size.
					std::span<uint8_t> buffer_view((uint8_t*)(buffer.data() + header.mipOffsets[i]), header.mipSizes[i]);

					int tmp_size = ((w + 3) / 4) * ((h + 3) / 4) * blockSize;
					if (video_support_compression) {
						glCompressedTexImage2DARB(GL_TEXTURE_2D, (GLint)i, format, w, h, 0, tmp_size, buffer_view.data());
					}
					else {
						switch (format) {
						case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
							DDSDecompressDXT1(buffer_view.data(), w, h, uncompressed_buffer.data());
							break;
						case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
							DDSDecompressDXT3(buffer_view.data(), w, h, uncompressed_buffer.data());
							break;
						case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
							DDSDecompressDXT5(buffer_view.data(), w, h, uncompressed_buffer.data());
							break;

						default:
							assert(false);
						}

						fn(i, w, h, uncompressed_buffer.data());
					}
				}
				else {
					break;
				}

				w >>= 1;
				h >>= 1;
			}
		}
		break;
		default:
			//TODO unsupported type
			assert(false);
			break;
		}
	}

	void BLPLoader::loadAll(callback_t fn) {
		bool has_mips = header.hasMips > 0;
		int32_t mip_max = has_mips ? 16 : 1;
		load(mip_max, std::move(fn));
	}

	//TODO confirm this does return best quality.
	void BLPLoader::loadFirst(callback_t fn) {
		load(1, std::move(fn));
	}

	Texture::Texture(GameFileUri uri) {
		this->fileUri = uri;
		id = Texture::INVALID_ID;
		width = 0;
		height = 0;
		compressed = false;
	}

	std::vector<uint8_t> Texture::getPixels(uint32_t format) {
		std::vector<uint8_t> buff;
		buff.resize(width * height * 4);

		glBindTexture(GL_TEXTURE_2D, id);
		glGetTexImage(GL_TEXTURE_2D, 0, format, GL_UNSIGNED_BYTE, buff.data());

		return buff;
	}

	std::shared_ptr<Texture> TextureManager::add(GameFileUri uri, GameFileSystem* fs) {

		for (auto it = textureMap.begin(); it != textureMap.end(); ++it) {
			if (auto temp = it->second.lock()) {
				if (temp->fileUri == uri) {
					return temp;
				}
			}
		}

		auto tex = std::shared_ptr<Texture>(new Texture(uri), [&](Texture* t) {
			remove(t->id);
			delete t;
		});

		assert(!textureMap.contains(tex->id));

		//// clear old texture memory from vid card
		glDeleteTextures(1, &tex->id);
		//// create new texture and put it in memory
		glGenTextures(1, &tex->id);

		loadBLP(tex.get(), fs);

		if (tex->id != Texture::INVALID_ID) {
			textureMap[tex->id] = tex;
			return tex;
		}

		Log::message("Unable to load texture: " + tex->fileUri.toString());
		return nullptr;
	}

	void TextureManager::remove(GLuint id)
	{
		if (glIsTexture(id)) {
			glDeleteTextures(1, &id);
		}

		textureMap.erase(id);
	}


	void TextureManager::loadBLP(Texture* tex, GameFileSystem* fs) {

		glBindTexture(GL_TEXTURE_2D, tex->id);

		ArchiveFile* file = fs->openFile(tex->fileUri);

		if (file == nullptr) {
			//throw FileIOException(tex->name.toStdString(), "cannot open file.");
			return;	//TODO make this throw! we should know if this errors, silent fail is bad. currently throwing exception looks to break some character loading.
		}

		auto file_guard = sg::make_scope_guard([&]() {
			fs->closeFile(file);
		});

		BLPLoader loader(file);
		const auto& header = loader.getHeader();

		tex->width = header.width;
		tex->height = header.height;
		tex->compressed = header.colorEncoding == BLPColorEncoding::COLOR_DXT;

		loader.loadAll([](int32_t mip_index, uint32_t w, uint32_t h, void* buffer_data) {
			glTexImage2D(GL_TEXTURE_2D, (GLint)mip_index, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer_data);
		});

		/*
		// TODO: Add proper support for mipmaps
		if (hasmipmaps) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		} else {
		*/
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//}
	}

	void CharacterTextureBuilder::setBaseLayer(const GameFileUri& textureUri) {
		baseLayers.clear();
		pushBaseLayer(textureUri);
	}

	void CharacterTextureBuilder::pushBaseLayer(const GameFileUri& textureUri) {
		baseLayers.push_back(textureUri);
		Log::message("Texture base: " + textureUri.toString());
	}

	void CharacterTextureBuilder::addLayer(const GameFileUri& textureUri, CharacterRegion region, int layer_index, BlendMode blend_mode)
	{
		Log::message("Texture layer: " + textureUri.toString());
		components.emplace_back(textureUri, region, layer_index, blend_mode);
	}

	std::shared_ptr<Texture> CharacterTextureBuilder::build(CharacterComponentTextureAdaptor* componentTextureAdaptor, TextureManager* manager, GameFileSystem* fs)
	{
		GLuint id = Texture::INVALID_ID;

		size_t x_scale = 1;
		size_t y_scale = 1;

		const auto REGION_PX_WIDTH = componentTextureAdaptor->getLayoutWidth();
		const auto REGION_PX_HEIGHT = componentTextureAdaptor->getLayoutHeight();
		const auto characterRegions = componentTextureAdaptor->getRegions();

		std::sort(components.begin(), components.end());

		std::vector<uint8_t> dest_buff(REGION_PX_WIDTH * x_scale * REGION_PX_HEIGHT * y_scale * 4, 0);

		const TextureBufferInfo buffer_info(dest_buff.data(), REGION_PX_WIDTH, REGION_PX_HEIGHT);

		//TODO not sure if the next 2 calls are needed?
		// clear old texture memory from vid card
		glDeleteTextures(1, &id);
		// create new texture and put it in memory
		glGenTextures(1, &id);

		{
			//handle base layer
			assert(baseLayers.size() > 0 && !baseLayers[0].isEmpty());

			for (const auto& bl : baseLayers) {
				mergeLayer(bl,
					manager,
					fs,
					buffer_info,
					{ 0, 0, REGION_PX_WIDTH, REGION_PX_HEIGHT },
					BlendMode::BLIT
				);
			}
		}

		for (auto it = components.begin(); it != components.end(); ++it) {

			if (characterRegions.contains(it->region)) {	//TODO remove debug

				auto& coords = characterRegions.at(it->region);

				mergeLayer(
					it->uri,
					manager,
					fs,
					buffer_info,
					coords,
					it->blendMode
				);
			}
			
		}

		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, REGION_PX_WIDTH * x_scale, REGION_PX_HEIGHT * y_scale, 0, GL_RGBA, GL_UNSIGNED_BYTE, dest_buff.data());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		auto tex = std::shared_ptr<Texture>(new Texture(), [](Texture* t) {
			if (glIsTexture(t->id)) {
				glDeleteTextures(1, &t->id);
			}
			delete t;
		});

		tex->id = id;
		tex->width = REGION_PX_WIDTH * x_scale;
		tex->height = REGION_PX_HEIGHT * y_scale;
		tex->fileUri = 0ul;
		tex->compressed = false;

		return tex;
	}

	void CharacterTextureBuilder::mergeLayer(const GameFileUri& uri, TextureManager* manager, GameFileSystem* fs, const TextureBufferInfo& buffer_info, const CharacterRegionCoords& coords, BlendMode blendMode) {
		
		ArchiveFile* file = fs->openFile(uri);
		if (file == nullptr) {
			return;
		}

		auto guard = sg::make_scope_guard([&]() {
			fs->closeFile(file);
		});

		BLPLoader loader(file);

		loader.loadFirst([&](int32_t mip, uint32_t w, uint32_t h, void* buffer) {
			auto img = QImage((uchar*)buffer, w, h, QImage::Format::Format_RGBA8888);

			const QImage scaled = img.scaled(coords.sizeX, coords.sizeY, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

			const auto destPos = QPoint(coords.positionX, coords.positionY);
			QImage dest((uchar*)buffer_info.data, buffer_info.width, buffer_info.height, QImage::Format::Format_RGBA8888);
			QPainter painter(&dest);

			switch (blendMode) {
			case BlendMode::MULTIPLY:
				painter.setCompositionMode(QPainter::CompositionMode_Multiply);
				break;
			case BlendMode::OVERLAY:
				painter.setCompositionMode(QPainter::CompositionMode_Overlay);
				break;
			default:
				painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
				break;
			}

			painter.drawImage(destPos, scaled);
			painter.end();
		});
	}

}