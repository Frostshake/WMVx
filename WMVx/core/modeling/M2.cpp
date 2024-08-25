#include "../../stdafx.h"
#include "M2.h"
#include "../game/GameConstants.h"
#include "GenericModelAdaptors.h"

namespace core {

	template<bool Strict = false>
	struct ByteReader {
	public:
		ByteReader(uint8_t* data, size_t size) : pos(data), end(data + size), offset(0) {}
		~ByteReader() {
			assert(pos <= end);
		}

		template<typename T>
		requires (std::is_standard_layout_v<T> || std::is_scalar_v<T>)
		void operator>>(T& val) {
			using val_t = std::remove_reference_t<T>;

			if constexpr (Strict) {
				if ((pos + sizeof(T)) > end) {
					throw std::runtime_error("Attempted to read beyond end of byte buffer.");
				}
			}
			else {
				assert((pos + sizeof(T)) <= end);
			}


			if constexpr (std::is_scalar_v<val_t>) {
				val = *reinterpret_cast<val_t*>(pos);
			}
			else {
				memcpy(&val, pos, sizeof(T));
			}

			pos += sizeof(T);
			offset += sizeof(T);
		}

		template<typename T>
		requires (std::is_standard_layout_v<T> || std::is_scalar_v<T>)
		void operator>>(std::optional<T>& val) {
			T temp;
			*this >> temp;
			val.emplace(temp);
		}

		template<typename T>
		T read() {
			T val;
			*this >> val;
			return val;
		}

		size_t getOffset() const {
			return offset;
		}


	protected:
		uint8_t* pos;
		uint8_t* end;
		size_t offset;
	};

	std::pair<M2Header, size_t> M2Header::create(std::span<uint8_t> buffer)
	{

		M2Header header;
		ByteReader<true> reader(buffer.data(), buffer.size());

		reader >> header.magic;

		if (!signatureCompare(Signatures::MD20, header.magic)) {
			throw BadSignatureException("Invalid M2 magic.");
		}

		reader >> header.version;
		assert(header.version >= M2_VER_ALPHA);

		reader >> header.name;
		reader >> header.globalFlags;
		reader >> header.globalSequences;
		reader >> header.animations;
		reader >> header.animationLookup;

		if (header.version <= M2_VER_TBC_MAX) {
			reader >> header.playableAnimationLookup;
		}
		else {
			header.playableAnimationLookup = std::nullopt;
		}

		reader >> header.bones;
		reader >> header.keyBoneLookup;
		reader >> header.vertices;

		if (header.version <= M2_VER_TBC_MAX) {
			header.views = reader.read<M2Array>();
		}
		else {
			header.views = reader.read<uint32_t>();
		}

		reader >> header.colors;
		reader >> header.textures;
		reader >> header.transparency;

		if (header.version <= M2_VER_TBC_MAX) {
			reader >> header.textureFlipbooks;
		}
		else {
			header.textureFlipbooks = std::nullopt;
		}

		reader >> header.uvAnimations;
		reader >> header.textureReplace;
		reader >> header.renderFlags;
		reader >> header.boneLookup;
		reader >> header.textureLookup;
		reader >> header.textureUnits;
		reader >> header.transparencyLookup;
		reader >> header.uvAnimationLookup;

		reader >> header.boundingBox;
		reader >> header.boundingSphereRadius;
		reader >> header.collisionBox;
		reader >> header.collisionSphereRadius;

		reader >> header.boundingTriangles;
		reader >> header.boundingVertices;
		reader >> header.boundingNormals;

		reader >> header.attachments;
		reader >> header.attachmentLookup;
		reader >> header.events;
		reader >> header.lights;
		reader >> header.cameras;
		reader >> header.cameraLookup;
		reader >> header.ribbonEmitters;
		reader >> header.particleEmitters;

		if (header.version >= M2_VER_TBC_MIN && (header.globalFlags & GlobalFlags::USE_BLEND_MAP_OVERRIDES)) {
			reader >> header.blendMapOverrides;
		}
		else {
			header.blendMapOverrides = std::nullopt;
		}


		return std::make_pair(header, reader.getOffset());
	}

	M2Model::M2Model(GameFileSystem* fs, GameFileUri uri)
	{
		//TODO some features change despite the header version remaining the same (e.g legion chunks are still 272)
		//TODO add a 'expansion' enum to handle this, note this will need to handle both retail and classic variants.


		std::unique_ptr<ArchiveFile> file = fs->openFile(uri);
		if (file == nullptr) {
			throw FileIOException(uri.toString().toStdString(), "Cannot open model file.");
		}

		M2Signature file_sig;
		file->read(&file_sig, sizeof(file_sig));

		// non-chunked (legacy) files begin with MD20
		const bool is_chunked_file = !signatureCompare(Signatures::MD20, file_sig);

		std::vector<uint8_t> md2x_buffer;


		if (is_chunked_file) {
			_chunks = ChunkedFile2::getChunks(file.get());
			const auto md21_chunk = _chunks.find(Signatures::MD21);
			if (md21_chunk != _chunks.end()) {
				//MD21 chunk contains the content of the old MD20 format.
				md2x_buffer.resize(md21_chunk->second.size);
				file->read(md2x_buffer.data(), md21_chunk->second.size, md21_chunk->second.offset);
			}
			else {
				throw BadStructureException("Unable to find MD21 chunk.");
			}
		}
		else {
			const auto file_size = file->getFileSize();
			md2x_buffer.resize(file_size);
			file->read(md2x_buffer.data(), file_size, 0);
		}

		{
			auto [header, header_bytes] = M2Header::create(std::span(md2x_buffer));
			this->_header = std::move(header);
		}

		std::map<size_t, ChunkedFile2> animFiles;	//archive files keyed by animation_id

		ChunkedFile2 skeleton_file = [&]() -> ChunkedFile2 {
			const auto skid_chunk = _chunks.find(Signatures::SKID);
			if (skid_chunk != _chunks.end()) {
				Chunks::SKID skid;
				assert(sizeof(skid) == skid_chunk->second.size);
				file->read(&skid, skid_chunk->second.size, skid_chunk->second.offset);

				return ChunkedFile2(fs->openFile(skid.skeletonFileId));
			}

			return ChunkedFile2(nullptr);
		}();

		if (skeleton_file.file) {
			processSkelFiles(fs, skeleton_file.file.get(), skeleton_file.chunks, [this](ArchiveFile* f, const ChunkedFile2::Chunks& c, int32_t file_index) {
				const auto sks1_chunk = c.find(Signatures::SKS1);
				if (sks1_chunk != c.end()) {
					Chunks::SKS1 sks1;
					assert(sizeof(sks1) <= sks1_chunk->second.size);
					f->read(&sks1, sizeof(sks1), sks1_chunk->second.offset);
					
					if (sks1.globalSequences.size) {
						decltype(globalSequences) temp_sequences;
						temp_sequences.resize(sks1.globalSequences.size);
						f->read(temp_sequences.data(), sizeof(uint32_t) * sks1.globalSequences.size, sks1_chunk->second.offset + sks1.globalSequences.offset);
						std::move(temp_sequences.begin(), temp_sequences.end(), std::back_inserter(globalSequences));
					}
				}

				const auto ska1_chunk = c.find(Signatures::SKA1);
				if (ska1_chunk != c.end()) {
					Chunks::SKA1 ska1;
					assert(sizeof(ska1) <= ska1_chunk->second.size);
					f->read(&ska1, sizeof(ska1), ska1_chunk->second.offset);

		//			//TODO it appears that attahcments should be overritten, not appended? - check.

					if (ska1.attachments.size) {
						const auto matched = M2_VER_RANGE_LIST<
								M2_VER_RANGE::FROM(M2_VER_WOTLK),
								M2_VER_RANGE::UPTO(M2_VER_WOTLK - 1)
							>::match(
								_header.version,
								[&]<M2_VER_RANGE R>() {
									std::vector< ModelAttachmentM2<R>> attach_buffer;
									attach_buffer.resize(ska1.attachments.size);
									f->read(attach_buffer.data(), ska1.attachments.size * sizeof(ModelAttachmentM2<R>), ska1.attachments.offset);

									for (auto& el : attach_buffer) {
										attachmentDefinitionAdaptors.push_back(
											std::make_unique<GenericModelAttachmentDefinitionAdaptor<R>>(std::move(el))
										);
									}
								}
							);

						if (!matched) {
							throw BadStructureException("Unable to read attachment definitions (skel).");
						}
					}

					if (ska1.attachmentLookup.size) {
						decltype(attachmentLookups) temp_lookup;
						temp_lookup.resize(ska1.attachmentLookup.size);
						f->read(temp_lookup.data(), sizeof(uint16_t) * ska1.attachmentLookup.size, ska1_chunk->second.offset + ska1.attachmentLookup.offset);
						std::move(temp_lookup.begin(), temp_lookup.end(), std::back_inserter(attachmentLookups));
					}
				}
			});
		}
		else if(_header.globalSequences.size) {

			globalSequences.resize(_header.globalSequences.size);
			memcpy(globalSequences.data(), md2x_buffer.data() + _header.globalSequences.offset, sizeof(uint32_t) * _header.globalSequences.size);
			
			if (_header.attachments.size) {
				
				attachmentDefinitionAdaptors.reserve(_header.attachments.size);

				const auto matched = M2_VER_RANGE_LIST<
						M2_VER_RANGE::FROM(M2_VER_WOTLK), 
						M2_VER_RANGE::UPTO(M2_VER_WOTLK - 1)
					>::match(
						_header.version, 
						[&]<M2_VER_RANGE R>() {
							std::span<ModelAttachmentM2<R>> view(
								(ModelAttachmentM2<R>*)(md2x_buffer.data() + _header.attachments.offset),
								_header.attachments.size
							);
						
							for (auto& el : view) {
								attachmentDefinitionAdaptors.push_back(
									std::make_unique<GenericModelAttachmentDefinitionAdaptor<R>>(std::move(el))
								);
							}
						}
					);

				if (!matched) {
					throw BadStructureException("Unable to read attachment definitions.");
				}
			}
		
			if (_header.attachmentLookup.size) {
				attachmentLookups.resize(_header.attachmentLookup.size);
				memcpy(attachmentLookups.data(), md2x_buffer.data() + _header.attachmentLookup.offset, sizeof(uint16_t) * _header.attachmentLookup.size);
			}
		}

		////TODO check vanilla & woltk, they used to always check attachments even if globalsequences was empty.


		rawVertices.resize(_header.vertices.size);
		memcpy(rawVertices.data(), md2x_buffer.data() + _header.vertices.offset, sizeof(ModelVertexM2) * _header.vertices.size);

		vertices.resize(_header.vertices.size);
		normals.resize(_header.vertices.size);

		for (uint32_t i = 0; i < _header.vertices.size; i++) {
			vertices[i] = Vector3::yUpToZUp(rawVertices[i].position);
			normals[i] = Vector3::yUpToZUp(rawVertices[i].normal).normalize();
		}

		bounds.resize(_header.boundingVertices.size);
		memcpy(bounds.data(), md2x_buffer.data() + _header.boundingVertices.offset, sizeof(Vector3) * _header.boundingVertices.size);
		for (auto& bound : bounds) {
			bound = Vector3::yUpToZUp(bound);
		}

		boundTriangles.resize(_header.boundingTriangles.size);
		memcpy(boundTriangles.data(), md2x_buffer.data() + _header.boundingTriangles.offset, sizeof(uint16_t) * _header.boundingTriangles.size);

		if (_header.textures.size) {
			if (_header.textures.size > TEXTURE_MAX) {
				throw BadStructureException(uri.toString().toStdString(), "texture size exceeds max");
			}

			textureDefinitions.resize(_header.textures.size);
			memcpy(textureDefinitions.data(), md2x_buffer.data() + _header.textures.offset, sizeof(ModelTextureM2) * _header.textures.size);

			const auto txid_chunk = _chunks.find(Signatures::TXID);
			std::vector<Chunks::TXID> txids;
			if (txid_chunk != _chunks.end()) {
				txids.resize(txid_chunk->second.size / sizeof(Chunks::TXID));
				file->read(txids.data(), txid_chunk->second.size, txid_chunk->second.offset);
			}

			auto texdef_index = 0;
			for (const auto& texdef : textureDefinitions) {
				if (texdef.type == (uint32_t)TextureType::FILENAME) {
					if (txids.size() > 0) {
						//loadTexture(this, texdef_index, texdef, GameFileUri(txids[texdef_index].fileDataId));
					}
					else {
						QString textureName = QString(std::string((char*)md2x_buffer.data() + texdef.name.offset, texdef.name.size).c_str());
						//loadTexture(this, texdef_index, texdef, GameFileUri(textureName));
					}
				}
				else {
					//loadTexture(this, texdef_index, texdef, (GameFileUri::id_t)0);
				}
				texdef_index++;
			}

		//	//TODO handle load texture.
		}

	
		//std::visit(Overload{
		//	[&](uint32_t v) { 
		//		if (v == 0) {
		//			return;
		//		}

		//		//TODO

		//	},
		//	[&](const M2Array& v) {
		//		if (v.size == 0) {
		//			return;
		//		}

		//		//TODO
		//	}
		//}, _header.views);


		{
			//TODO combine 'processSkelFiles' calls if possible.
			std::vector<Chunks::AFID> afids;
			if (skeleton_file.file) {
				processSkelFiles(fs, skeleton_file.file.get(), skeleton_file.chunks, [this, &afids](ArchiveFile* f, const ChunkedFile2::Chunks& c, int32_t file_index) {
					const auto sks1_chunk = c.find(Signatures::SKS1);
					if (sks1_chunk != c.end()) {
						Chunks::SKS1 sks1;
						assert(sizeof(sks1) <= sks1_chunk->second.size);
						f->read(&sks1, sizeof(sks1), sks1_chunk->second.offset);


						// note animation sequnces can replace parent items from the child file, and when doing so they are not in the same order or size.
						// logic seems to be - replace, in place the sequence record, otherwise append.

						if (sks1.animations.size) {
							const auto matched = M2_VER_RANGE_LIST<
									M2_VER_RANGE::FROM(M2_VER_WOTLK),
									M2_VER_RANGE::UPTO(M2_VER_WOTLK - 1)
								>::match(
									_header.version,
									[&]<M2_VER_RANGE R>() {
										std::vector<AnimationSequenceM2<R>> anim_buffer;
										anim_buffer.resize(sks1.animations.size);
										f->read(anim_buffer.data(), sks1.animations.size * sizeof(AnimationSequenceM2<R>), sks1.animations.offset);

										for (auto& seq : anim_buffer) {
											auto existing_seq = std::find_if(animationSequenceAdaptors.begin(), animationSequenceAdaptors.end(), [&seq](const auto/*ModelAnimationSequenceAdaptor*/& existing) -> bool {
												return existing->getId() == seq.id && existing->getVariationId() == seq.variationId;
											});

											auto ptr = std::make_unique<GenericModelAnimationSequenceAdaptor<R>>(std::move(seq));

											if (existing_seq != animationSequenceAdaptors.end()) {
												*existing_seq = std::move(ptr);
											}
											else {
												animationSequenceAdaptors.push_back(std::move(ptr));
											}
										}
									}
								);

							if (!matched) {
								throw BadStructureException("Unable to read animation definitions (skel).");
							}
						}

						if (sks1.animationLookup.size) {
							decltype(animationLookups) temp_lookups;
							temp_lookups.resize(sks1.animationLookup.size);
							f->read(temp_lookups.data(), sizeof(uint16_t)* sks1.animationLookup.size, sks1_chunk->second.offset + sks1.animationLookup.offset);
							std::move(temp_lookups.begin(), temp_lookups.end(), std::back_inserter(animationLookups));
						}
					}

					const auto afid_chunk = c.find(Signatures::AFID);
					if (afid_chunk != c.end()) {
						std::vector<Chunks::AFID> temp_afids;
						temp_afids.resize(afid_chunk->second.size / sizeof(Chunks::AFID));
						f->read(temp_afids.data(), afid_chunk->second.size, afid_chunk->second.offset);
						std::move(temp_afids.begin(), temp_afids.end(), std::back_inserter(afids));
					}
				});
			}
			else {

				if (_header.animations.size) {
					animationSequenceAdaptors.reserve(_header.animations.size);

					const auto matched = M2_VER_RANGE_LIST<
						M2_VER_RANGE::FROM(M2_VER_WOTLK),
						M2_VER_RANGE::UPTO(M2_VER_WOTLK - 1)
					>::match(
						_header.version,
						[&]<M2_VER_RANGE R>() {
							std::span<AnimationSequenceM2<R>> view(
								(AnimationSequenceM2<R>*)(md2x_buffer.data() + _header.animations.offset),
								_header.animations.size
							);

							for (auto& el : view) {
								animationSequenceAdaptors.push_back(
									std::make_unique<GenericModelAnimationSequenceAdaptor<R>>(std::move(el))
								);
							}
						}
					);

					if (!matched) {
						throw BadStructureException("Unable to read animation definitions.");
					}

					const auto afid_chunk = _chunks.find(Signatures::AFID);
					if (afid_chunk != _chunks.end()) {
						afids.resize(afid_chunk->second.size / sizeof(Chunks::AFID));
						file->read(afids.data(), afid_chunk->second.size, afid_chunk->second.offset);
					}
				}

				if (_header.animationLookup.size) {
					animationLookups.resize(_header.animationLookup.size);
					memcpy(animationLookups.data(), md2x_buffer.data() + _header.animationLookup.offset, sizeof(uint16_t) * _header.animationLookup.size);
				}
			}

			size_t anim_index = 0;
			for (const auto& anim_seq : animationSequenceAdaptors) {
				const auto mainAnimId = anim_seq->getId();
				const auto subAnimId = anim_seq->getVariationId();

				std::unique_ptr<ArchiveFile> animFile = nullptr;

				if (afids.size() > 0) {
					auto matching_afid = std::find_if(afids.begin(), afids.end(), [&](const Chunks::AFID& afid) {
						return mainAnimId == afid.animationId &&
							subAnimId == afid.variationId &&
							afid.fileId > 0;
						});

					if (matching_afid != afids.end()) {
						animFile = std::move(fs->openFile(matching_afid->fileId));
					}
				}
				else {
					//TODO lookup via file name.
					//const QString& fileName = getFileInfo().path;
					//QString animName = fileName.mid(0, fileName.lastIndexOf('.')) + QString("%1-%2.anim").arg(QString::number(mainAnimId), 4, '0').arg(QString::number(subAnimId), 2, '0');
					//animFile.reset((CascFile*)fs->openFile(animName).release());
				}

				if (animFile != nullptr) {
					animFiles.emplace(anim_index, ChunkedFile2(std::move(animFile)));
				}

				anim_index++;
			}
		}

		bool match_anim_type = M2_VER_RANGE_LIST<
			M2_VER_RANGE::FROM(M2_VER_WOTLK),
			M2_VER_RANGE::UPTO(M2_VER_WOTLK - 1)
		>::match(
			_header.version,
			[&]<M2_VER_RANGE R>() {

				if (_header.colors.size) {
					std::span<ModelColorM2<R>> def_view(
						(ModelColorM2<R>*)(md2x_buffer.data()+ _header.colors.offset),
						_header.colors.size
					);

					colorAdaptors.reserve(def_view.size());

					for (const auto& color_def : def_view) {
						//TODO
					}
				}

				if (_header.transparency.size) {
					//TODO
				}
			}
		);


		if (!match_anim_type) {
			throw BadStructureException("Unable to read animation structures.");
		}


		if (_header.keyBoneLookup.size) {
			keyBoneLookup.resize(_header.keyBoneLookup.size);
			memcpy(keyBoneLookup.data(), md2x_buffer.data() + _header.keyBoneLookup.offset, sizeof(int16_t) * _header.keyBoneLookup.size);
		}

		{
			//TODO bones
		}

		if (_header.uvAnimations.size) {
			//TODO
		}

		if (_header.particleEmitters.size) {
			//TODO
		}

		if (_header.ribbonEmitters.size) {
			//TODO
		}

		if (_header.events.size) {
			//events
		}

		if (_header.cameras.size) {
			//cameras
		}

		if (_header.lights.size) {
			//lights
		}
	}
}