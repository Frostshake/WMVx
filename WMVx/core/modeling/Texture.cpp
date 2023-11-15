#include "../../stdafx.h"
#include "Texture.h"
#include <bitset>
#include "../utility/Logger.h"
#include "../utility/Exceptions.h"

#include <ximage.h> 
#include "../../ddslib.h"

namespace core {

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

		//TODO TIDY CODE

		bool video_support_compression = false; //TODO detect / config

		glBindTexture(GL_TEXTURE_2D, tex->id);

		ArchiveFile* file = fs->openFile(tex->fileUri);

		if (file == nullptr) {
			//throw FileIOException(tex->name.toStdString(), "cannot open file.");
			return;	//TODO make this throw! we should know if this errors, silent fail is bad. currently throwing exception looks to break some character loading.
		}

		auto bufferSize = file->getFileSize();
		auto buffer = std::vector<uint8_t>(bufferSize);
		file->read(buffer.data(), bufferSize);
		fs->closeFile(file);

		BLPHeader header;
		memcpy(&header, buffer.data(), sizeof(BLPHeader));

		std::string signature((char*)header.signature, sizeof(header.signature));
		if (signature != "BLP2") {
			throw BadSignatureException(tex->fileUri.toString().toStdString(), signature, "BLP2");
		}

		tex->width = header.width;
		tex->height = header.height;

		bool has_mips = header.hasMips > 0;
		int32_t mip_max = has_mips ? 16 : 1;

		uint32_t w = header.width;
		uint32_t h = header.height;

		switch (header.colorEncoding) {
		case BLPColorEncoding::COLOR_PALETTE:
		{
			tex->compressed = false;

			std::span<uint32_t> pal_view((uint32_t*)(buffer.data() + sizeof(BLPHeader)), sizeof(uint32_t) * 256);

			bool has_alpha = header.alphaSize != 0;

			//TODO CHECK LOGIC
			for (auto i = 0; i < mip_max; i++) {
				if (w == 0) w = 1;
				if (h == 0) h = 1;

				auto out_buffer = std::vector<uint32_t>(tex->width * tex->height);

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
								else if(header.alphaSize == 4) {
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

					glTexImage2D(GL_TEXTURE_2D, (GLint)i, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, out_buffer.data());
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

			tex->compressed = true;

			auto uncompressed_buffer = std::vector<uint8_t>();

			if (!video_support_compression) {
				uncompressed_buffer.resize(tex->width * tex->height * 4);
			}

			for (auto i = 0; i < mip_max; i++) {
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

						glTexImage2D(GL_TEXTURE_2D, (GLint)i, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, uncompressed_buffer.data());

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
		baseLayer = textureUri;
	}

	void CharacterTextureBuilder::addLayer(const GameFileUri& textureUri, CharacterRegion region, int layer_index, BlendMode blend_mode)
	{
		Component c;
		c.uri = textureUri;
		c.region = region;
		c.layerIndex = layer_index;
		c.blendMode = blend_mode;

		Log::message("Texture layer: " + textureUri.toString());

		components.push_back(c);
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

		//TODO not sure if the next 2 calls are needed?
		// clear old texture memory from vid card
		glDeleteTextures(1, &id);
		// create new texture and put it in memory
		glGenTextures(1, &id);

		{
			//handle base layer
			assert(!baseLayer.isEmpty());

			mergeLayer(baseLayer, 
				manager, 
				fs, 
				dest_buff, 
				REGION_PX_WIDTH, 
				{0, 0, REGION_PX_WIDTH, REGION_PX_HEIGHT}
			);
		}

		for (auto it = components.begin(); it != components.end(); ++it) {
			auto& coords = characterRegions.at(it->region);

			mergeLayer(
				it->uri,
				manager,
				fs,
				dest_buff,
				REGION_PX_WIDTH,
				coords
			);
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

	void CharacterTextureBuilder::mergeLayer(const GameFileUri& uri, TextureManager* manager, GameFileSystem* fs, std::vector<uint8_t>& dest_buff, int32_t dest_width, CharacterRegionCoords coords) {
		auto temptex = manager->add(uri, fs);

		size_t x_scale = 1;
		size_t y_scale = 1;

		if (temptex->width == 0 || temptex->height == 0 || temptex->id == Texture::INVALID_ID) {
			return;
		}

		std::vector<uint8_t> temp_buff;

		if (temptex->width != coords.sizeX || temptex->height != coords.sizeY) {
			//TODO TIDY MESSY CODE!
			temp_buff = temptex->getPixels(GL_BGRA_EXT);
			std::unique_ptr<CxImage> newImage = std::make_unique<CxImage>(CxImage(0));
			newImage->AlphaCreate();	// Create the alpha layer
			newImage->IncreaseBpp(32);	// set image to 32bit 
			newImage->CreateFromArray(temp_buff.data(), temptex->width, temptex->height, 32, (temptex->width * 4), false);
			newImage->Resample(coords.sizeX, coords.sizeY, 0); // 0: hight quality, 1: normal quality
			temp_buff.resize(0);

			long out_size = coords.sizeX * coords.sizeY * 4;
			temp_buff.resize(out_size);
			BYTE* b = nullptr;
			newImage->Encode2RGBA(b, out_size, false);

			memcpy(temp_buff.data(), b, out_size);
			delete b;
		}
		else {
			temp_buff = temptex->getPixels();
		}

		for (auto y = 0, dy = coords.positionY; y < coords.sizeY; y++, dy++) {
			for (auto x = 0, dx = coords.positionX; x < coords.sizeX; x++, dx++) {
				//TODO tidy code
				uint8_t* src = temp_buff.data() + y * coords.sizeX * 4 + x * 4;
				uint8_t* dest = dest_buff.data() + dy * dest_width * x_scale * 4 + dx * 4;

				// this is slow and ugly but I don't care
				float r = src[3] / 255.0f;
				float ir = 1.0f - r;
				// zomg RGBA?
				dest[0] = (unsigned char)(dest[0] * ir + src[0] * r);
				dest[1] = (unsigned char)(dest[1] * ir + src[1] * r);
				dest[2] = (unsigned char)(dest[2] * ir + src[2] * r);
				dest[3] = 255;
			}
		}
	}

}