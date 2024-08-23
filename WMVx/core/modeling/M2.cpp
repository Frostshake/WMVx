#include "../../stdafx.h"
#include "M2.h"
#include "../game/GameConstants.h"

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
		assert(header.version > 256);

		reader >> header.globalFlags;
		reader >> header.globalSequences;
		reader >> header.animations;
		reader >> header.animationLookup;

		if (header.version <= 260) {
			reader >> header.playableAnimationLookup;
		}
		else {
			header.playableAnimationLookup = std::nullopt;
		}

		reader >> header.bones;
		reader >> header.keyBoneLookup;
		reader >> header.vertices;

		if (header.version <= 260) {
			header.views = reader.read<M2Array>();
		}
		else {
			header.views = reader.read<uint32_t>();
		}

		reader >> header.colors;
		reader >> header.textures;
		reader >> header.transparency;

		if (header.version <= 260) {
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

		if (header.version >= 260 && (header.globalFlags & GlobalFlags::USE_BLEND_MAP_OVERRIDES)) {
			reader >> header.blendMapOverrides;
		}
		else {
			header.blendMapOverrides = std::nullopt;
		}


		return std::make_pair(header, reader.getOffset());
	}
	ChunkedFile2::Chunks ChunkedFile2::getChunks(ArchiveFile* file)
	{
		std::map<M2Signature, Chunk> result;
		auto to_read = file->getFileSize();
		size_t offset = 0;
		struct {
			M2Signature id;
			uint32_t size;
		} header;

		while (to_read > sizeof(header)) {
			file->read(&header, sizeof(header), offset);
			offset += sizeof(header);

			assert(!result.contains(header.id));
			result.emplace(header.id, Chunk{
					header.id,
					header.size,
					offset
				});

			offset += header.size;
			to_read -= (header.size + sizeof(header));
		}

		return result;
	}

	M2Model::M2Model(GameFileSystem* fs, GameFileUri uri)
	{
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

		std::unique_ptr<ArchiveFile> skeleton_file = [&]() -> std::unique_ptr<ArchiveFile> {
			const auto skid_chunk = _chunks.find(Signatures::SKID);
			if (skid_chunk != _chunks.end()) {
				Chunks::SKID skid;
				assert(sizeof(skid) == skid_chunk->second.size);
				file->read(&skid, skid_chunk->second.size, skid_chunk->second.offset);

				return fs->openFile(skid.skeletonFileId);
			}

			return nullptr;
		}();
		ChunkedFile2::Chunks skeleton_chunks;

		if (skeleton_file) {
			skeleton_chunks = ChunkedFile2::getChunks(skeleton_file.get());
		}


		//if (skeleton_file) {
		//	processSkelFiles(fs, skeleton_file.get(), skeleton_chunks, [this](ArchiveFile* f, const ChunkedFile2::Chunks& c, int32_t file_index) {
		//		const auto sks1_chunk = c.find(Signatures::SKS1);
		//		if (sks1_chunk != c.end()) {
		//			Chunks::SKS1 sks1;
		//			assert(sizeof(sks1) <= sks1_chunk->second.size);
		//			f->read(&sks1, sizeof(sks1), sks1_chunk->second.offset);
		//			
		//			if (sks1.globalSequences.size) {
		//				decltype(globalSequences) temp_sequences;
		//				temp_sequences.resize(sks1.globalSequences.size);
		//				f->read(temp_sequences.data(), sizeof(uint32_t) * sks1.globalSequences.size, sks1_chunk->second.offset + sks1.globalSequences.offset);
		//				std::move(temp_sequences.begin(), temp_sequences.end(), std::back_inserter(globalSequences));
		//			}
		//		}

		//		const auto ska1_chunk = c.find(Signatures::SKA1);
		//		if (ska1_chunk != c.end()) {
		//			Chunks::SKA1 ska1;
		//			assert(sizeof(ska1) <= ska1_chunk->second.size);
		//			f->read(&ska1, sizeof(ska1), ska1_chunk->second.offset);

		//			//TODO it appears that attahcments should be overritten, not appended? - check.

		//			if (ska1.attachments.size) {
		//				decltype(attachments) temp_attach;
		//				temp_attach.reserve(ska1.attachments.size);

		//				//TODO
		//				//TODO needs to somehow determine the size of the attachment record to know the size of the buffer.
		//			}

		//			if (ska1.attachmentLookup.size) {
		//				decltype(attachmentLookups) temp_lookup;
		//				temp_lookup.resize(ska1.attachmentLookup.size);
		//				f->read(temp_lookup.data(), sizeof(uint16_t) * ska1.attachmentLookup.size, ska1_chunk->second.offset + ska1.attachmentLookup.offset);
		//				std::move(temp_lookup.begin(), temp_lookup.end(), std::back_inserter(attachmentLookups));
		//			}
		//		}
		//	});
		//}
		//else if(_header.globalSequences.size) {

		//	globalSequences.resize(_header.globalSequences.size);
		//	memcpy(globalSequences.data(), md2x_buffer.data() + _header.globalSequences.offset, sizeof(uint32_t) * _header.globalSequences.size);
		//	
		//	if (_header.attachments.size) {
		//		attachments.reserve(_header.attachments.size);
		//		ByteReader reader(md2x_buffer.data() + _header.attachments.offset, md2x_buffer.size() - _header.attachments.offset);
		//		for (auto i = 0; i < _header.attachments.size; i++) {
		//			M2AttachmentDef attach;
		//			reader >> attach.id;
		//			reader >> attach.bone;
		//			reader >> attach.unknown;

		//			if (_header.version < 264) {
		//				attach.animateAttached = reader.read<M2AnimationBlockDefLegacy>();
		//			}
		//			else {
		//				attach.animateAttached = reader.read<M2AnimationBlockDef>();
		//			}

		//			attachments.push_back(std::move(attach));
		//		}
		//	}
		//
		//	if (_header.attachmentLookup.size) {
		//		attachmentLookups.resize(_header.attachmentLookup.size);
		//		memcpy(attachmentLookups.data(), md2x_buffer.data() + _header.attachmentLookup.offset, sizeof(uint16_t) * _header.attachmentLookup.size);
		//	}
		//}

		////TODO check vanilla & woltk, they used to always check attachments even if globalsequences was empty.


		//rawVertices.resize(_header.vertices.size);
		//memcpy(rawVertices.data(), md2x_buffer.data() + _header.vertices.offset, sizeof(ModelVertexM2) * _header.vertices.size);

		//vertices.resize(_header.vertices.size);
		//normals.resize(_header.vertices.size);

		//for (uint32_t i = 0; i < _header.vertices.size; i++) {
		//	vertices[i] = Vector3::yUpToZUp(rawVertices[i].position);
		//	normals[i] = Vector3::yUpToZUp(rawVertices[i].normal).normalize();
		//}

		//bounds.resize(_header.boundingVertices.size);
		//memcpy(bounds.data(), md2x_buffer.data() + _header.boundingVertices.offset, sizeof(Vector3) * _header.boundingVertices.size);
		//for (auto& bound : bounds) {
		//	bound = Vector3::yUpToZUp(bound);
		//}

		//boundTriangles.resize(_header.boundingTriangles.size);
		//memcpy(boundTriangles.data(), md2x_buffer.data() + _header.boundingTriangles.offset, sizeof(uint16_t) * _header.boundingTriangles.size);

		//if (_header.textures.size) {
		//	if (_header.textures.size > TEXTURE_MAX) {
		//		throw BadStructureException(uri.toString().toStdString(), "texture size exceeds max");
		//	}

		//	textureDefinitions.resize(_header.textures.size);
		//	memcpy(textureDefinitions.data(), md2x_buffer.data() + _header.textures.offset, sizeof(ModelTextureM2) * _header.textures.size);

		//	const auto txid_chunk = _chunks.find(Signatures::TXID);
		//	std::vector<Chunks::TXID> txids;
		//	if (txid_chunk != _chunks.end()) {
		//		txids.resize(txid_chunk->second.size / sizeof(Chunks::TXID));
		//		file->read(txids.data(), txid_chunk->second.size, txid_chunk->second.offset);
		//	}

		//	auto texdef_index = 0;
		//	for (const auto& texdef : textureDefinitions) {
		//		if (texdef.type == (uint32_t)TextureType::FILENAME) {
		//			if (txids.size() > 0) {
		//				//loadTexture(this, texdef_index, texdef, GameFileUri(txids[texdef_index].fileDataId));
		//			}
		//			else {
		//				QString textureName = QString(std::string((char*)md2x_buffer.data() + texdef.name.offset, texdef.name.size).c_str());
		//				//loadTexture(this, texdef_index, texdef, GameFileUri(textureName));
		//			}
		//		}
		//		else {
		//			//loadTexture(this, texdef_index, texdef, (GameFileUri::id_t)0);
		//		}
		//		texdef_index++;
		//	}

		//	//TODO handle load texture.
		//}

	
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


		//{
		//	std::vector<Chunks::AFID> afids;
		//	if (skeleton_file) {
		//		processSkelFiles(fs, skeleton_file.get(), skeleton_chunks, [this, &afids](ArchiveFile* f, const ChunkedFile2::Chunks& c, int32_t file_index) {
		//			const auto sks1_chunk = c.find(Signatures::SKS1);
		//			if (sks1_chunk != c.end()) {
		//				Chunks::SKS1 sks1;
		//				assert(sizeof(sks1) <= sks1_chunk->second.size);
		//				f->read(&sks1, sizeof(sks1), sks1_chunk->second.offset);


		//				// note animation sequnces can replace parent items from the child file, and when doing so they are not in the same order or size.
		//				// logic seems to be - replace, in place the sequence record, otherwise append.

		//				if (sks1.animations.size) {
		//					//TODO
		//				}
		//			}

		//			const auto afid_chunk = c.find(Signatures::AFID);
		//			if (afid_chunk != c.end()) {
		//				std::vector<Chunks::AFID> temp_afids;
		//				temp_afids.resize(afid_chunk->second.size / sizeof(Chunks::AFID));
		//				f->read(temp_afids.data(), afid_chunk->second.size, afid_chunk->second.offset);
		//				std::move(temp_afids.begin(), temp_afids.end(), std::back_inserter(afids));
		//			}
		//		});
		//	}
		//	else {

		//		if (_header.animations.size) {
		//			animationSequences.reserve(_header.animations.size);
		//			ByteReader reader(md2x_buffer.data() + _header.animations.offset, md2x_buffer.size() - _header.animations.offset);
		//			//TODO ideally need to workout number of bytes per section from _header.animations.size.;

		//			for (auto i = 0; i < _header.animations.size; i++) {
		//				M2AnimationSequenceDef seq;
		//				reader >> seq.id;
		//				reader >> seq.variationId;

		//				if (_header.version <= 260) {
		//					seq.duration = reader.read<M2AnimationSequenceDef::LegacyTimestamps>();
		//				}
		//				else {
		//					seq.duration = reader.read<uint32_t>();
		//				}

		//				reader >> seq.movespeed;
		//				reader >> seq.flags;
		//				reader >> seq.frequency;
		//				reader >> seq.unused;
		//				reader >> seq.minimumRepitions;
		//				reader >> seq.maximumRepitions;
		//				reader >> seq.blendTime;
		//				reader >> seq.bounds;
		//				reader >> seq.boundsRadius;
		//				reader >> seq.nextAnimationId;
		//				reader >> seq.aliasNextId;

		//				animationSequences.push_back(std::move(seq));
		//			}

		//			const auto afid_chunk = _chunks.find(Signatures::AFID);
		//			if (afid_chunk != _chunks.end()) {
		//				afids.resize(afid_chunk->second.size / sizeof(Chunks::AFID));
		//				file->read(afids.data(), afid_chunk->second.size, afid_chunk->second.offset);
		//			}
		//		}

		//		if (_header.animationLookup.size) {
		//			animationLookups.resize(_header.animationLookup.size);
		//			memcpy(animationLookups.data(), md2x_buffer.data() + _header.animationLookup.offset, sizeof(uint16_t) * _header.animationLookup.size);
		//		}
		//	}


		//	//TODO handle loading anim files.
		//}

		if (_header.colors.size) {
			//TODO
		}

		if (_header.transparency.size) {
			//TODO
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