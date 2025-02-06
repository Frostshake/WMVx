#include "../../stdafx.h"
#include "M2.h"
#include "../game/GameConstants.h"
#include "../utility/Overload.h"
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

		if (header.version >= M2_VER_TBC_MIN && (header.globalFlags & ModelGlobalFlags::USE_BLEND_MAP_OVERRIDES)) {
			reader >> header.blendMapOverrides;
		}
		else {
			header.blendMapOverrides = std::nullopt;
		}


		return std::make_pair(header, reader.getOffset());
	}

	void M2Loader::load(M2Data* m2, GameFileSystem* fs, const GameFileUri& uri)
	{
		m2->fileInfo = fs->asInfo(uri);

		//TODO some features change despite the header version remaining the same (e.g legion chunks are still 272)
		//TODO add a 'expansion' enum to handle this, note this will need to handle both retail and classic variants.

		std::unique_ptr<ArchiveFile> file = fs->openFile(uri);
		if (file == nullptr) {
			throw FileIOException(uri.toString().toStdString(), "Cannot open model file.");
		}


		file->read(&m2->_magic, sizeof(m2->_magic));
		const bool is_md20 = signatureCompare(Signatures::MD20, m2->_magic);
		const bool is_md21 = signatureCompare(Signatures::MD21, m2->_magic);
		
		if (!is_md20 && !is_md21) {
			throw BadStructureException("File does not contain valid m2 signature.");
		}

		// non-chunked (legacy) files begin with MD20
		const bool is_chunked_file = is_md21;

		std::vector<uint8_t> md2x_buffer;

		if (is_chunked_file) {
			//TODO need better method for determining chunked file.
			m2->_chunks = ChunkedFile::getChunks(file.get());
			const auto md21_chunk = m2->_chunks.find(Signatures::MD21);
			if (md21_chunk != m2->_chunks.end()) {
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
			m2->_header = std::move(header);
		}

		// no-op placeholder for animated fix functions.
		auto no_fix = [](auto&& val) { return val; };

		m2->globalSequences = std::make_shared<std::vector<uint32_t>>();

		std::map<size_t, ChunkedFile> animFiles;	//archive files keyed by animation_id

		ChunkedFile skeleton_file = [&]() -> ChunkedFile {
			const auto skid_chunk = m2->_chunks.find(Signatures::SKID);
			if (skid_chunk != m2->_chunks.end()) {
				Chunks::SKID skid;
				assert(sizeof(skid) == skid_chunk->second.size);
				file->read(&skid, skid_chunk->second.size, skid_chunk->second.offset);

				return ChunkedFile(fs->openFile(skid.skeletonFileId));
			}

			return ChunkedFile(nullptr);
			}();

		if (skeleton_file.file) {
			processSkelFiles(fs, skeleton_file.file.get(), skeleton_file.chunks, [&](ArchiveFile* f, const ChunkedFile::Chunks& c, int32_t file_index) {
				const auto sks1_chunk = c.find(Signatures::SKS1);
				if (sks1_chunk != c.end()) {
					Chunks::SKS1 sks1;
					assert(sizeof(sks1) <= sks1_chunk->second.size);
					f->read(&sks1, sizeof(sks1), sks1_chunk->second.offset);

					if (sks1.globalSequences.size) {
						decltype(m2->globalSequences)::element_type temp_sequences;
						temp_sequences.resize(sks1.globalSequences.size);
						f->read(temp_sequences.data(), sizeof(uint32_t) * sks1.globalSequences.size, sks1_chunk->second.offset + sks1.globalSequences.offset);
						std::move(temp_sequences.begin(), temp_sequences.end(), std::back_inserter(*(m2->globalSequences)));
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
							m2->_header.version,
							[&]<M2_VER_RANGE R>() {
							std::vector< ModelAttachmentM2<R>> attach_buffer;
							attach_buffer.resize(ska1.attachments.size);
							f->read(attach_buffer.data(), ska1.attachments.size * sizeof(ModelAttachmentM2<R>), ska1_chunk->second.offset + ska1.attachments.offset);

							for (auto& el : attach_buffer) {
								m2->attachmentDefinitionAdaptors.push_back(
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
						decltype(m2->attachmentLookups) temp_lookup;
						temp_lookup.resize(ska1.attachmentLookup.size);
						f->read(temp_lookup.data(), sizeof(uint16_t) * ska1.attachmentLookup.size, ska1_chunk->second.offset + ska1.attachmentLookup.offset);
						std::move(temp_lookup.begin(), temp_lookup.end(), std::back_inserter(m2->attachmentLookups));
					}
				}
				});
		}
		else if (m2->_header.globalSequences.size) {

			m2->globalSequences->resize(m2->_header.globalSequences.size);
			memcpy(m2->globalSequences->data(), md2x_buffer.data() + m2->_header.globalSequences.offset, sizeof(uint32_t) * m2->_header.globalSequences.size);

			if (m2->_header.attachments.size) {

				m2->attachmentDefinitionAdaptors.reserve(m2->_header.attachments.size);

				const auto matched = M2_VER_RANGE_LIST<
					M2_VER_RANGE::FROM(M2_VER_WOTLK),
					M2_VER_RANGE::UPTO(M2_VER_WOTLK - 1)
				>::match(
					m2->_header.version,
					[&]<M2_VER_RANGE R>() {
					std::span<ModelAttachmentM2<R>> view(
						(ModelAttachmentM2<R>*)(md2x_buffer.data() + m2->_header.attachments.offset),
						m2->_header.attachments.size
					);

					for (auto& el : view) {
						m2->attachmentDefinitionAdaptors.push_back(
							std::make_unique<GenericModelAttachmentDefinitionAdaptor<R>>(std::move(el))
						);
					}
				}
				);

				if (!matched) {
					throw BadStructureException("Unable to read attachment definitions.");
				}
			}

			if (m2->_header.attachmentLookup.size) {
				m2->attachmentLookups.resize(m2->_header.attachmentLookup.size);
				memcpy(m2->attachmentLookups.data(), md2x_buffer.data() + m2->_header.attachmentLookup.offset, sizeof(uint16_t) * m2->_header.attachmentLookup.size);
			}
		}

		////TODO check vanilla & woltk, they used to always check attachments even if globalsequences was empty.


		m2->rawVertices.resize(m2->_header.vertices.size);
		memcpy(m2->rawVertices.data(), md2x_buffer.data() + m2->_header.vertices.offset, sizeof(ModelVertexM2) * m2->_header.vertices.size);

		m2->vertices.resize(m2->_header.vertices.size);
		m2->normals.resize(m2->_header.vertices.size);

		for (uint32_t i = 0; i < m2->_header.vertices.size; i++) {
			m2->vertices[i] = Vector3::yUpToZUp(m2->rawVertices[i].position);
			m2->normals[i] = Vector3::yUpToZUp(m2->rawVertices[i].normal).normalize();
		}

		m2->bounds.resize(m2->_header.boundingVertices.size);
		memcpy(m2->bounds.data(), md2x_buffer.data() + m2->_header.boundingVertices.offset, sizeof(Vector3) * m2->_header.boundingVertices.size);
		for (auto& bound : m2->bounds) {
			bound = Vector3::yUpToZUp(bound);
		}

		m2->boundTriangles.resize(m2->_header.boundingTriangles.size);
		memcpy(m2->boundTriangles.data(), md2x_buffer.data() + m2->_header.boundingTriangles.offset, sizeof(uint16_t) * m2->_header.boundingTriangles.size);

		if (m2->_header.textures.size) {
			if (m2->_header.textures.size > TEXTURE_MAX) {
				throw BadStructureException(uri.toString().toStdString(), "texture size exceeds max");
			}

			m2->textureDefinitions.resize(m2->_header.textures.size);
			memcpy(m2->textureDefinitions.data(), md2x_buffer.data() + m2->_header.textures.offset, sizeof(ModelTextureM2) * m2->_header.textures.size);

			const auto txid_chunk = m2->_chunks.find(Signatures::TXID);
			std::vector<Chunks::TXID> txids;
			if (txid_chunk != m2->_chunks.end()) {
				txids.resize(txid_chunk->second.size / sizeof(Chunks::TXID));
				file->read(txids.data(), txid_chunk->second.size, txid_chunk->second.offset);
			}

			size_t texdef_index = 0;
			for (const auto& texdef : m2->textureDefinitions) {
				if (texdef.type == (uint32_t)TextureType::FILENAME) {
					if (txids.size() > 0) {
						textures.emplace_back(TextureLoadDef{ texdef_index, texdef, GameFileUri(txids[texdef_index].fileDataId) });
					}
					else {
						QString textureName = QString(std::string((char*)md2x_buffer.data() + texdef.name.offset, texdef.name.size).c_str());
						textures.emplace_back(TextureLoadDef{ texdef_index, texdef, GameFileUri(textureName) });
					}
				}
				else {
					textures.emplace_back(TextureLoadDef{ texdef_index, texdef, (GameFileUri::id_t)0 });
				}
				texdef_index++;
			}
		}

		{

			auto load_indices = [&]<M2_VER_RANGE R>(const ModelViewM2<R>&view, std::span<uint8_t> skin_buffer) {
				std::span<uint16_t> indexLookup((uint16_t*)(skin_buffer.data() + view.indices.offset), view.indices.size);
				std::span<uint16_t> triangles((uint16_t*)(skin_buffer.data() + view.triangles.offset), view.triangles.size);

				m2->indices.resize(view.triangles.size);

				for (uint32_t i = 0; i < view.triangles.size; i++) {
					m2->indices[i] = indexLookup[triangles[i]];
				}
			};

			auto load_render_passes = [&]<M2_VER_RANGE R>(const ModelViewM2<R>&view, std::span<uint8_t> skin_buffer) {

				std::span<ModelTextureUnitM2> modelTextureUnits((ModelTextureUnitM2*)(skin_buffer.data() + view.textureUnits.offset), view.textureUnits.size);
				std::span<ModelRenderFlagsM2> render_flags_source((ModelRenderFlagsM2*)(md2x_buffer.data() + m2->_header.renderFlags.offset), m2->_header.renderFlags.size);
				std::span<uint16_t> textureLookup((uint16_t*)(md2x_buffer.data() + m2->_header.textureLookup.offset), m2->_header.textureLookup.size);
				std::span<uint16_t> textureAnimLookup((uint16_t*)(md2x_buffer.data() + m2->_header.uvAnimationLookup.offset), m2->_header.uvAnimationLookup.size);
				std::span<uint16_t> transparencyLookup((uint16_t*)(md2x_buffer.data() + m2->_header.transparencyLookup.offset), m2->_header.transparencyLookup.size);


				renderPasses.reserve(view.textureUnits.size);
				for (uint32_t i = 0; i < view.textureUnits.size; i++) {

					const auto& mtu = modelTextureUnits[i];
					const auto& rf = render_flags_source[mtu.renderFlagsIndex];
					ModelRenderPass pass(rf, mtu);

					//TODO TIDY

					auto& geoset = m2->geosetAdaptors[pass.geosetIndex];

					pass.indexStart = geoset->getTriangleStart();
					pass.indexCount = geoset->getTriangleCount();
					pass.vertexStart = geoset->getVertexStart();
					pass.vertexEnd = pass.vertexStart + geoset->getVertexCount();

					pass.tex = textureLookup[mtu.textureId];
					pass.opacity = transparencyLookup[mtu.transparencyIndex];

					pass.trans = pass.blendmode > 0 && pass.opacity > 0;

					pass.p = geoset->getCenterMass().z;

					pass.swrap = (m2->textureDefinitions[pass.tex].flags & TextureFlag::WRAPX) != 0;
					pass.twrap = (m2->textureDefinitions[pass.tex].flags & TextureFlag::WRAPY) != 0;

					if ((m2->textureDefinitions[pass.tex].flags & TextureFlag::STATIC) == 0) {
						pass.texanim = textureAnimLookup[mtu.textureAnimationId];
					}

					renderPasses.push_back(std::move(pass));

				}


				//TODO do render passes need special sorting? see wmv source
			};

			std::visit(Overload{
				[&](uint32_t v) {
					if (v == 0) {
						return;
					}

					// skins are in skin files. ( >= WOTLK)

					assert(m2->_header.version >= M2_VER_WOTLK);

					//TODO handle multiple views

					std::unique_ptr<ArchiveFile> skinFile;
					QString skinName;

					const auto sfid_chunk = m2->_chunks.find(Signatures::SFID);
					if (sfid_chunk != m2->_chunks.end()) {
						std::vector<uint32_t> skinFileIds(sfid_chunk->second.size / sizeof(uint32_t), 0);
						file->read(skinFileIds.data(), sfid_chunk->second.size, sfid_chunk->second.offset);

						if (skinFileIds.size() > 0) {
							skinName = QString::number(skinFileIds[0]);
							skinFile = fs->openFile(skinFileIds[0]);
						}
					}
					else {
						skinName = m2->getFileInfo().path;
						skinName = GameFileUri::removeExtension(skinName) + "00" + ".skin";
						skinFile = fs->openFile(skinName);
					}

					if (skinFile) {
						const auto skinSize = skinFile->getFileSize();
						auto skinBuffer = std::vector<uint8_t>(skinSize);
						skinFile->read(skinBuffer.data(), skinSize);
						skinFile.reset();


						bool match_view_type = M2_VER_RANGE_LIST<
							M2_VER_RANGE::FROM(M2_VER_CATA_MIN),
							M2_VER_RANGE(M2_VER_WOTLK, M2_VER_CATA_MIN - 1)
						>::match(
							m2->_header.version,
							[&]<M2_VER_RANGE R>() {


							ModelViewM2<R>* view = (ModelViewM2<R>*)skinBuffer.data();

							if (!signatureCompare(Signatures::SKIN, *reinterpret_cast<M2Signature*>(&view->id))) {
								throw BadSignatureException("Invalid SKIN id.");
							}

							load_indices(*view, std::span(skinBuffer));

							std::vector<ModelGeosetM2<R>> geosets(view->submeshes.size);
							memcpy(geosets.data(), skinBuffer.data() + view->submeshes.offset, sizeof(ModelGeosetM2<R>) * view->submeshes.size);

							m2->geosetAdaptors.reserve(geosets.size());

							if (m2->_header.version >= M2_VER_LEGION_PLUS) {
								// from looking at old wmv source, its appears the problem of 'triangle start' started around legion.
								// sometimes triangle start can overflow int16, to overcome this, count manually and override. 
								// (unsure if there is a more reliable way within the data?)

								uint32_t custom_triangle_start = 0;
								for (auto& geoset : geosets) {
									uint32_t temp = geoset.triangleCount;
									m2->geosetAdaptors.push_back(std::make_unique<OverridableModelGeosetAdaptor<R>>(std::move(geoset), custom_triangle_start));
									custom_triangle_start += temp;
								}

							}
							else {
								for (auto& geoset : geosets) {
									m2->geosetAdaptors.push_back(std::make_unique<GenericModelGeosetAdaptor<R>>(std::move(geoset)));
								}
							}

							load_render_passes(*view, std::span(skinBuffer));

						});


						if (!match_view_type) {
							throw BadStructureException("Unable to read view structures.");
						}
					}

				},
				[&](const M2Array& v) {
					if (v.size == 0) {
						return;
					}

					//skins are in md20 buffer (<= TBC)

					bool match_view_type = M2_VER_RANGE_LIST<
						M2_VER_RANGE::UPTO(M2_VER_TBC_MAX)
					>::match(
						m2->_header.version,
						[&]<M2_VER_RANGE R>() {

							std::span<ModelViewM2<R>> views((ModelViewM2<R>*)(md2x_buffer.data() + v.offset), v.size);

							//TODO handle multiple views.

							ModelViewM2<R>& view = views[0];

							load_indices(view, std::span(md2x_buffer));

							bool match_geoset_type = M2_VER_RANGE_LIST<
								M2_VER_RANGE::FROM(M2_VER_TBC_MIN),
								M2_VER_RANGE::UPTO(M2_VER_TBC_MIN - 1)
							>::match(
								m2->_header.version,
								[&]<M2_VER_RANGE R2>() {

									std::vector<ModelGeosetM2<R2>> geosets(view.submeshes.size);
									memcpy(geosets.data(), md2x_buffer.data() + view.submeshes.offset, sizeof(ModelGeosetM2<R2>) * view.submeshes.size);

									for (auto& geoset : geosets) {
										m2->geosetAdaptors.push_back(std::make_unique<GenericModelGeosetAdaptor<R2>>(std::move(geoset)));
									}
								});

							if (!match_geoset_type) {
								throw BadStructureException("Unable to read geosets structures.");
							}

							load_render_passes(view, std::span(md2x_buffer));
						});


					if (!match_view_type) {
						throw BadStructureException("Unable to read view structures.");
					}
				}
				}, m2->_header.views);

		}


		{
			//TODO combine 'processSkelFiles' calls if possible.
			std::vector<Chunks::AFID> afids;
			if (skeleton_file.file) {
				processSkelFiles(fs, skeleton_file.file.get(), skeleton_file.chunks, [&](ArchiveFile* f, const ChunkedFile::Chunks& c, int32_t file_index) {
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
								m2->_header.version,
								[&]<M2_VER_RANGE R>() {
								std::vector<AnimationSequenceM2<R>> anim_buffer;
								anim_buffer.resize(sks1.animations.size);
								f->read(anim_buffer.data(), sks1.animations.size * sizeof(AnimationSequenceM2<R>), sks1_chunk->second.offset + sks1.animations.offset);

								for (auto& seq : anim_buffer) {
									auto existing_seq = std::find_if(m2->animationSequenceAdaptors.begin(), m2->animationSequenceAdaptors.end(), [&seq](const auto/*ModelAnimationSequenceAdaptor*/& existing) -> bool {
										return existing->getId() == seq.id && existing->getVariationId() == seq.variationId;
										});

									auto ptr = std::make_unique<GenericModelAnimationSequenceAdaptor<R>>(std::move(seq));

									if (existing_seq != m2->animationSequenceAdaptors.end()) {
										*existing_seq = std::move(ptr);
									}
									else {
										m2->animationSequenceAdaptors.push_back(std::move(ptr));
									}
								}
							}
							);

							if (!matched) {
								throw BadStructureException("Unable to read animation definitions (skel).");
							}
						}

						if (sks1.animationLookup.size) {
							decltype(m2->animationLookups) temp_lookups;
							temp_lookups.resize(sks1.animationLookup.size);
							f->read(temp_lookups.data(), sizeof(uint16_t) * sks1.animationLookup.size, sks1_chunk->second.offset + sks1.animationLookup.offset);
							std::move(temp_lookups.begin(), temp_lookups.end(), std::back_inserter(m2->animationLookups));
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

				if (m2->_header.animations.size) {
					m2->animationSequenceAdaptors.reserve(m2->_header.animations.size);

					const auto matched = M2_VER_RANGE_LIST<
						M2_VER_RANGE::FROM(M2_VER_WOTLK),
						M2_VER_RANGE::UPTO(M2_VER_WOTLK - 1)
					>::match(
						m2->_header.version,
						[&]<M2_VER_RANGE R>() {
						std::span<AnimationSequenceM2<R>> view(
							(AnimationSequenceM2<R>*)(md2x_buffer.data() + m2->_header.animations.offset),
							m2->_header.animations.size
						);

						for (auto& el : view) {
							m2->animationSequenceAdaptors.push_back(
								std::make_unique<GenericModelAnimationSequenceAdaptor<R>>(std::move(el))
							);
						}
					}
					);

					if (!matched) {
						throw BadStructureException("Unable to read animation definitions.");
					}

					const auto afid_chunk = m2->_chunks.find(Signatures::AFID);
					if (afid_chunk != m2->_chunks.end()) {
						afids.resize(afid_chunk->second.size / sizeof(Chunks::AFID));
						file->read(afids.data(), afid_chunk->second.size, afid_chunk->second.offset);
					}
				}

				if (m2->_header.animationLookup.size) {
					m2->animationLookups.resize(m2->_header.animationLookup.size);
					memcpy(m2->animationLookups.data(), md2x_buffer.data() + m2->_header.animationLookup.offset, sizeof(uint16_t) * m2->_header.animationLookup.size);
				}
			}

			size_t anim_index = 0;
			for (const auto& anim_seq : m2->animationSequenceAdaptors) {
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
				else if (m2->_chunks.size() == 0) {
					const QString& fileName = m2->getFileInfo().path;
					QString animName = fileName.mid(0, fileName.lastIndexOf('.')) + QString("%1-%2.anim").arg(QString::number(mainAnimId), 4, '0').arg(QString::number(subAnimId), 2, '0');
					animFile = std::move(fs->openFile(animName));
				}

				if (animFile != nullptr) {
					const bool is_chunked_anim_file = skeleton_file.file || (m2->_header.globalFlags & ModelGlobalFlags::CHUNKED_ANIM_0x2000);
					animFiles.emplace(anim_index, ChunkedFile(std::move(animFile), is_chunked_anim_file));
				}

				anim_index++;
			}
		}

		if (m2->_header.keyBoneLookup.size) {
			m2->keyBoneLookup.resize(m2->_header.keyBoneLookup.size);
			memcpy(m2->keyBoneLookup.data(), md2x_buffer.data() + m2->_header.keyBoneLookup.offset, sizeof(int16_t) * m2->_header.keyBoneLookup.size);
		}

		bool match_anim_type = M2_VER_RANGE_LIST<
			M2_VER_RANGE::FROM(M2_VER_WOTLK),
			M2_VER_RANGE::UPTO(M2_VER_WOTLK - 1)
		>::match(
			m2->_header.version,
			[&]<M2_VER_RANGE R>() {

			if (m2->_header.colors.size) {
				std::span<ModelColorM2<R>> def_view(
					(ModelColorM2<R>*)(md2x_buffer.data() + m2->_header.colors.offset),
					m2->_header.colors.size
				);

				m2->colorAdaptors.reserve(def_view.size());

				for (auto& color_def : def_view) {
					auto color_data = AnimationBlock<Vector3, R>::fromDefinition(color_def.color, md2x_buffer, animFiles);
					auto opacity_data = AnimationBlock<int16_t, R>::fromDefinition(color_def.opacity, md2x_buffer, animFiles);

					auto adaptor = std::make_unique<GenericModelColorAdaptor<R>>(
						AnimatedValue<Vector3, R>::make(std::move(color_data), m2->globalSequences, no_fix),
						AnimatedValue<float, R>::template make<int16_t, ShortToFloat>(std::move(opacity_data), m2->globalSequences, no_fix)
					);

					m2->colorAdaptors.push_back(std::move(adaptor));
				}
			}

			if (m2->_header.transparency.size) {
				std::span<ModelTransparencyM2<R>> def_view(
					(ModelTransparencyM2<R>*)(md2x_buffer.data() + m2->_header.transparency.offset),
					m2->_header.transparency.size
				);

				m2->transparencyAdaptors.reserve(def_view.size());

				for (auto& trans_def : def_view) {
					auto trans_data = AnimationBlock<int16_t, R>::fromDefinition(trans_def.transparency, md2x_buffer, animFiles);

					auto adaptor = std::make_unique<GenericModelTransparencyAdaptor<R>>(
						AnimatedValue<float, R>::template make<int16_t, ShortToFloat>(std::move(trans_data), m2->globalSequences, no_fix)
					);

					m2->transparencyAdaptors.push_back(std::move(adaptor));
				}
			}

			if (m2->_header.uvAnimations.size) {
				std::span<TextureAnimationM2<R>> def_view(
					(TextureAnimationM2<R>*)(md2x_buffer.data() + m2->_header.uvAnimations.offset),
					m2->_header.uvAnimations.size
				);

				m2->textureAnimationAdaptors.reserve(def_view.size());

				for (auto& uv_anim_def : def_view) {
					auto trans = AnimationBlock<Vector3, R>::fromDefinition(uv_anim_def.translation, md2x_buffer, animFiles);
					auto rot = AnimationBlock<Vector3, R>::fromDefinition(uv_anim_def.rotation, md2x_buffer, animFiles);	//TODO this should be quaternion?
					auto scale = AnimationBlock<Vector3, R>::fromDefinition(uv_anim_def.scale, md2x_buffer, animFiles);

					auto adaptor = std::make_unique<GenericModelTextureAnimationAdaptor<R>>(
						AnimatedValue<Vector3, R>::make(std::move(trans), m2->globalSequences, no_fix),
						AnimatedValue<Vector3, R>::make(std::move(rot), m2->globalSequences, no_fix),
						AnimatedValue<Vector3, R>::make(std::move(scale), m2->globalSequences, no_fix)
					);

					m2->textureAnimationAdaptors.push_back(std::move(adaptor));
				}
			}
		}
		);


		if (!match_anim_type) {
			throw BadStructureException("Unable to read animation structures.");
		}

		bool match_bone_type = M2_VER_RANGE_LIST<
			M2_VER_RANGE::FROM(M2_VER_WOTLK),
			M2_VER_RANGE(M2_VER_TBC_MIN, M2_VER_WOTLK - 1),
			M2_VER_RANGE::UPTO(M2_VER_TBC_MIN - 1)
		>::match(
			m2->_header.version,
			[&]<M2_VER_RANGE R>() {
				{
					//bones

					auto load_bones = [&](std::vector<ModelBoneM2<R>>&& bonesDefinitions, const std::vector<uint8_t>& src_buffer) {
						if (bonesDefinitions.size()) {
							m2->boneAdaptors.reserve(m2->boneAdaptors.size() + bonesDefinitions.size());

							auto fix_quaternion = [](const Quaternion& q) {
								return Quaternion(-q.x, -q.z, q.y, q.w);
								};

							const std::span<uint8_t> buffer_view(*const_cast<std::decay_t<decltype(src_buffer)>*>(&src_buffer));

							for (ModelBoneM2<R>& boneDef : bonesDefinitions) {
								auto trans_data = AnimationBlock<Vector3, R>::fromDefinition(boneDef.translation, buffer_view, animFiles);
								auto scale_data = AnimationBlock<Vector3, R>::fromDefinition(boneDef.scale, buffer_view, animFiles);

								auto trans_value = AnimatedValue<Vector3, R>::make(std::move(trans_data), m2->globalSequences, Vector3::yUpToZUp);
								auto scale_value = AnimatedValue<Vector3, R>::make(std::move(scale_data), m2->globalSequences, [](const Vector3& v) {
									return Vector3(v.x, v.z, v.y);
									});

								if (m2->_header.version <= M2_VER_VANILLA_MAX) {
									auto rot_data = AnimationBlock<Quaternion, R>::fromDefinition(boneDef.rotation, buffer_view, animFiles);
									auto adaptor = std::make_unique<GenericModelBoneAdaptor<R>>(
										std::move(boneDef),
										std::move(trans_value),
										AnimatedValue<Quaternion, R>::make(std::move(rot_data), m2->globalSequences, fix_quaternion),
										std::move(scale_value)
									);

									m2->boneAdaptors.push_back(std::move(adaptor));

								}
								else {
									auto rot_data = AnimationBlock<PACK_QUATERNION, R>::fromDefinition(boneDef.rotation, buffer_view, animFiles);

									auto adaptor = std::make_unique<GenericModelBoneAdaptor<R>>(
										std::move(boneDef),
										std::move(trans_value),
										AnimatedValue<Quaternion, R>::template make<PACK_QUATERNION, Quat16ToQuat32>(std::move(rot_data), m2->globalSequences, fix_quaternion),
										std::move(scale_value)
									);

									m2->boneAdaptors.push_back(std::move(adaptor));
								}
							}
						}
						};

					if (skeleton_file.file) {
						processSkelFiles(fs, skeleton_file.file.get(), skeleton_file.chunks, [&](ArchiveFile* f, const ChunkedFile::Chunks& c, int32_t file_index) {
							const auto skb1_chunk = c.find(Signatures::SKB1);
							if (skb1_chunk != c.end()) {
								Chunks::SKB1 skb1;
								assert(sizeof(skb1) <= skb1_chunk->second.size);
								f->read(&skb1, sizeof(skb1), skb1_chunk->second.offset);

								if (skb1.keyBoneLookup.size) {
									decltype(m2->keyBoneLookup) temp_lookup;
									temp_lookup.resize(skb1.keyBoneLookup.size);
									f->read(temp_lookup.data(), sizeof(int16_t) * skb1.keyBoneLookup.size, skb1_chunk->second.offset + skb1.keyBoneLookup.offset);
									std::move(temp_lookup.begin(), temp_lookup.end(), std::back_inserter(m2->keyBoneLookup));
								}

								auto temp_bonesDefinitions = std::vector<ModelBoneM2<R>>(skb1.bones.size);
								f->read(temp_bonesDefinitions.data(), sizeof(ModelBoneM2<R>) * skb1.bones.size, skb1_chunk->second.offset + skb1.bones.offset);

								const auto skel_size = f->getFileSize();
								auto temp_src_buffer = std::vector<uint8_t>(skel_size - skb1_chunk->second.offset);
								f->read(temp_src_buffer.data(), temp_src_buffer.size(), skb1_chunk->second.offset);

								load_bones(std::move(temp_bonesDefinitions), temp_src_buffer);
							}
							});
					}
					else {
						if (m2->_header.keyBoneLookup.size) {
							m2->keyBoneLookup.resize(m2->_header.keyBoneLookup.size);
							memcpy(m2->keyBoneLookup.data(), md2x_buffer.data() + m2->_header.keyBoneLookup.offset, sizeof(int16_t) * m2->_header.keyBoneLookup.size);
						}

						auto bonesDefinitions = std::vector<ModelBoneM2<R>>(m2->_header.bones.size);
						memcpy(bonesDefinitions.data(), md2x_buffer.data() + m2->_header.bones.offset, sizeof(ModelBoneM2<R>) * m2->_header.bones.size);


						load_bones(std::move(bonesDefinitions), md2x_buffer);
					}
				}
		}
		);

		if (!match_anim_type) {
			throw BadStructureException("Unable to read bone structures.");
		}

		if (m2->_header.particleEmitters.size) {
			bool match_particle_type = M2_VER_RANGE_LIST<
				M2_VER_RANGE::FROM(M2_VER_CATA_MIN),
				M2_VER_RANGE(M2_VER_WOTLK, M2_VER_CATA_MIN - 1)//,
				//TODO further investigation needed on the structures before being safe to enable
				//M2_VER_RANGE(M2_VER_TBC_MAX, M2_VER_WOTLK -1),
				//M2_VER_RANGE::UPTO(M2_VER_TBC_MAX - 1)
			>::match(
				m2->_header.version,
				[&]<M2_VER_RANGE R>() {
					auto particleDefinitons = std::vector<ModelParticleEmitterM2<R>>(m2->_header.particleEmitters.size);
					memcpy(particleDefinitons.data(), md2x_buffer.data() + m2->_header.particleEmitters.offset, sizeof(ModelParticleEmitterM2<R>) * m2->_header.particleEmitters.size);



					for (auto& particleDef : particleDefinitons) {

						auto speed = AnimationBlock<float, R>::fromDefinition(particleDef.emissionSpeed, md2x_buffer, animFiles);
						auto variation = AnimationBlock<float, R>::fromDefinition(particleDef.speedVariation, md2x_buffer, animFiles);
						auto spread = AnimationBlock<float, R>::fromDefinition(particleDef.verticalRange, md2x_buffer, animFiles);
						auto lat = AnimationBlock<float, R>::fromDefinition(particleDef.horizontalRange, md2x_buffer, animFiles);
						auto gravity = AnimationBlock<float, R>::fromDefinition(particleDef.gravity, md2x_buffer, animFiles);
						auto lifespan = AnimationBlock<float, R>::fromDefinition(particleDef.lifespan, md2x_buffer, animFiles);
						auto rate = AnimationBlock<float, R>::fromDefinition(particleDef.emissionRate, md2x_buffer, animFiles);
						auto areal = AnimationBlock<float, R>::fromDefinition(particleDef.emissionAreaLength, md2x_buffer, animFiles);
						auto areaw = AnimationBlock<float, R>::fromDefinition(particleDef.emissionAreaWidth, md2x_buffer, animFiles);
						auto deacceleration = AnimationBlock<float, R>::fromDefinition(particleDef.zSource, md2x_buffer, animFiles);
						auto enabled = AnimationBlock<float, R>::fromDefinition(particleDef.enabledIn, md2x_buffer, animFiles);


						 auto adaptor = std::make_unique<GenericModelParticleEmitterAdaptor<R>>();


						 adaptor->definition = particleDef;

						 adaptor->speed = AnimatedValue<float, R>::make(std::move(speed), m2->globalSequences, no_fix);
						 adaptor->variation = AnimatedValue<float, R>::make(std::move(variation), m2->globalSequences, no_fix);
						 adaptor->spread = AnimatedValue<float, R>::make(std::move(spread), m2->globalSequences, no_fix);
						 adaptor->lat = AnimatedValue<float, R>::make(std::move(lat), m2->globalSequences, no_fix);
						 adaptor->gravity = AnimatedValue<float, R>::make(std::move(gravity), m2->globalSequences, no_fix);
						 adaptor->lifespan = AnimatedValue<float, R>::make(std::move(lifespan), m2->globalSequences, no_fix);
						 adaptor->rate = AnimatedValue<float, R>::make(std::move(rate), m2->globalSequences, no_fix);
						 adaptor->areal = AnimatedValue<float, R>::make(std::move(areal), m2->globalSequences, no_fix);
						 adaptor->areaw = AnimatedValue<float, R>::make(std::move(areaw), m2->globalSequences, no_fix);
						 adaptor->deacceleration = AnimatedValue<float, R>::make(std::move(deacceleration), m2->globalSequences, no_fix);
						 adaptor->enabled = AnimatedValue<float, R>::make(std::move(enabled), m2->globalSequences, no_fix);
						 adaptor->rem = 0;

						 
						 constexpr auto has_colors_as_block = 
							 std::is_same_v<decltype(particleDef.color), FakeAnimationBlockM2> &&
							 std::is_same_v<decltype(particleDef.opacity), FakeAnimationBlockM2> &&
							 std::is_same_v<decltype(particleDef.scale), FakeAnimationBlockM2>;


						 if constexpr (has_colors_as_block) {
							 //TODO check logic here, wtf is it all
							std::span<Vector3> colors2((Vector3*)(md2x_buffer.data() + particleDef.color.keys.offset), 3);
							for (size_t i = 0; i < 3; i++) {
								float opacity = *(short*)(md2x_buffer.data() + particleDef.opacity.keys.offset + i * 2);
								adaptor->colors[i] = Vector4(colors2[i].x / 255.0f, colors2[i].y / 255.0f, colors2[i].z / 255.0f, opacity / 32767.0f);
								adaptor->sizes[i] = (*(float*)(md2x_buffer.data() + particleDef.scale.keys.offset + i * sizeof(Vector2))) * particleDef.burstMultiplier;
							}
						 }
						 else {
							 //TODO check.
							 for (size_t i = 0; i < 3; i++) {
								 adaptor->colors[i] = Vector4(
									particleDef.colorValues[i].red  /255.0f,
									particleDef.colorValues[i].green / 255.0f,
									particleDef.colorValues[i].blue / 255.0f,
									particleDef.colorValues[i].alpha / 255.0f
								 );
								 adaptor->sizes[i] = particleDef.scalesValues[i] * particleDef.burstMultiplier;
							 }
						 }

						 switch (particleDef.emitterType) {
						 case ParticleEmitterType::PLANE:
							 adaptor->generator = &ParticleFactory::plane;
							 break;
						 case ParticleEmitterType::SPHERE:
							 adaptor->generator = &ParticleFactory::sphere;
							 break;
						 default:
							 assert(false); //TODO handle all types
						 }

						 auto initTile = [&adaptor](Vector2* tc, int num) {
							 const auto order = adaptor->getParticleType() > 0 ? -1 : 0;
							 const auto rows = std::max(1, (int)adaptor->definition.textureDimensionRows);
							 const auto cols = std::max(1, (int)adaptor->definition.textureDimensionColumns);

							 Vector2 otc[4];
							 Vector2 a, b;
							 int x = num % cols;
							 int y = num / cols;
							 a.x = x * (1.0f / cols);
							 b.x = (x + 1) * (1.0f / cols);
							 a.y = y * (1.0f / rows);
							 b.y = (y + 1) * (1.0f / rows);

							 otc[0] = a;
							 otc[2] = b;
							 otc[1].x = b.x;
							 otc[1].y = a.y;
							 otc[3].x = a.x;
							 otc[3].y = b.y;

							 for (size_t i = 0; i < 4; i++) {
								 tc[(i + 4 - order) & 3] = otc[i];
							 }
						 };

						 for (size_t i = 0; i < std::max(1, particleDef.textureDimensionColumns * particleDef.textureDimensionRows); i++) {
							 ModelParticleEmitterAdaptor::TexCoordSet tc;
							 initTile(tc.texCoord, (int)i);
							 adaptor->tiles.push_back(std::move(tc));
						 }

						 adaptor->position = Vector3::yUpToZUp(particleDef.position);

						 m2->particleAdaptors.push_back(std::move(adaptor));
					}

				});

			//assert(match_particle_type);
		}

		if (m2->_header.ribbonEmitters.size) {
			bool match_ribbon_type = M2_VER_RANGE_LIST<
				M2_VER_RANGE::FROM(M2_VER_WOTLK),
				M2_VER_RANGE::UPTO(M2_VER_WOTLK - 1)
			>::match(
				m2->_header.version,
				[&]<M2_VER_RANGE R>() {

				auto ribbonDefintions = std::vector<ModelRibbonEmitterM2<R>>(m2->_header.ribbonEmitters.size);
				memcpy(ribbonDefintions.data(), md2x_buffer.data() + m2->_header.ribbonEmitters.offset, sizeof(ModelRibbonEmitterM2<R>) * m2->_header.ribbonEmitters.size);

				for (auto& ribbon_def : ribbonDefintions) {
					auto color_data = AnimationBlock<Vector3, R>::fromDefinition(ribbon_def.color, md2x_buffer, animFiles);
					auto alpha_data = AnimationBlock<int16_t, R>::fromDefinition(ribbon_def.alpha, md2x_buffer, animFiles);
					auto above_data = AnimationBlock<float, R>::fromDefinition(ribbon_def.heightAbove, md2x_buffer, animFiles);
					auto below_data = AnimationBlock<float, R>::fromDefinition(ribbon_def.heightBelow, md2x_buffer, animFiles);

					std::vector<uint16_t> textures(ribbon_def.textures.size);
					memcpy(textures.data(), md2x_buffer.data() + ribbon_def.textures.offset, sizeof(uint16_t) * ribbon_def.textures.size);

					auto adaptor = std::make_unique<GenericModelRibbonEmitter<R>>(
						std::move(ribbon_def),
						AnimatedValue<Vector3, R>::make(std::move(color_data), m2->globalSequences, no_fix),
						AnimatedValue<float, R>::template make<int16_t, ShortToFloat>(std::move(alpha_data), m2->globalSequences, no_fix),
						AnimatedValue<float, R>::make(std::move(above_data), m2->globalSequences, no_fix),
						AnimatedValue<float, R>::make(std::move(below_data), m2->globalSequences, no_fix)
					);

					adaptor->textures = std::move(textures);

					m2->ribbonAdaptors.push_back(std::move(adaptor));
				}
			});

			assert(match_ribbon_type);
		}

		if (m2->_header.events.size) {
			//events
		}

		if (m2->_header.cameras.size) {
			//cameras
		}

		if (m2->_header.lights.size) {
			//lights
		}

	}

	
}