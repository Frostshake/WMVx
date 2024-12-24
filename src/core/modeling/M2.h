#pragma once
#include "M2Definitions.h"
#include "../filesystem/GameFileSystem.h"
#include "../utility/Exceptions.h"
#include "../utility/Color.h"
#include "Animation.h"
#include "ModelAdaptors.h"
#include "ModelPathInfo.h"
#include <memory>
#include <optional>
#include <span>
#include <variant>
#include <cstdint>
#include <utility>

namespace core {

	struct ModelRenderPass;
	struct TextureLoadDef;


	struct M2Header {
	public:

		// Reads the header from the buffer, returns the header and number of bytes read.
		static std::pair<M2Header, size_t> create(std::span<uint8_t> buffer);

		std::array<uint8_t, 4> magic;
		uint32_t version;
		M2Array name;
		std::underlying_type_t<ModelGlobalFlags> globalFlags;
		M2Array globalSequences;
		M2Array animations;
		M2Array animationLookup;
		std::optional<M2Array> playableAnimationLookup; // <= BC
		M2Array bones;
		M2Array keyBoneLookup;
		M2Array vertices;
		std::variant<uint32_t, M2Array> views;	//(M2Array <= BC) (uint32_t > BC)
		M2Array colors;
		M2Array textures;
		M2Array transparency;
		std::optional<M2Array> textureFlipbooks;
		M2Array uvAnimations;
		M2Array textureReplace;
		M2Array renderFlags;
		M2Array boneLookup;
		M2Array textureLookup;
		M2Array textureUnits;
		M2Array transparencyLookup;
		M2Array uvAnimationLookup;

		M2Box boundingBox;
		float boundingSphereRadius;
		M2Box collisionBox;
		float collisionSphereRadius;

		M2Array boundingTriangles;
		M2Array boundingVertices;
		M2Array boundingNormals;

		M2Array attachments;
		M2Array attachmentLookup;
		M2Array events;
		M2Array lights;
		M2Array cameras;
		M2Array cameraLookup;
		M2Array ribbonEmitters;
		M2Array particleEmitters;

		std::optional<M2Array> blendMapOverrides; // >= BC && USE_BLEND_MAP_OVERRIDES
	};

	class M2Data;

	class M2Loader {
	public:
		M2Loader(M2Data* m2, GameFileSystem* fs, const GameFileUri& uri) {
			load(m2, fs, uri);
		}

		void load(M2Data* m2, GameFileSystem* fs, const GameFileUri& uri);

		std::vector<ModelRenderPass> renderPasses;	
		std::vector<TextureLoadDef> textures;

	protected:

		// apply a callback for each skeleton file, starting the at top most parent.
		template<typename fn>
		void processSkelFiles(GameFileSystem* fs, ArchiveFile* file, const ChunkedFile::Chunks& chunks, fn callback, int32_t file_index = 0) {
			if (file != nullptr) {
				const auto skpd_chunk = chunks.find(Signatures::SKPD);
				if (skpd_chunk != chunks.end()) {
					Chunks::SKPD skpd;
					assert(sizeof(skpd) <= skpd_chunk->second.size);
					file->read(&skpd, sizeof(skpd), skpd_chunk->second.offset);

					if (skpd.parentSkelFileId) {
						std::unique_ptr<ArchiveFile> parent_file(fs->openFile(skpd.parentSkelFileId));
						if (parent_file != nullptr) {
							ChunkedFile::Chunks parent_chunks = ChunkedFile::getChunks(parent_file.get());
							processSkelFiles(fs, parent_file.get(), parent_chunks, callback, ++file_index);
						}
					}
				}

				callback(file, chunks, file_index);
			}
		}

		
	};


	class M2Data {
	public:

		const M2Header getHeader() const {
			return _header;
		}

		const GameFileInfo& getFileInfo() const {
			return fileInfo;
		}

		const std::vector<ModelGeosetAdaptor*>& getGeosetAdaptors() const {
			return reinterpret_cast<const std::vector<ModelGeosetAdaptor*>&>(geosetAdaptors);
		}

		const std::vector<ModelAnimationSequenceAdaptor*>& getModelAnimationSequenceAdaptors() const {
			return reinterpret_cast<const std::vector<ModelAnimationSequenceAdaptor*>&>(animationSequenceAdaptors);
		}

		const std::vector<ModelTextureAnimationAdaptor*>& getTextureAnimationAdaptors() const {
			return reinterpret_cast<const std::vector<ModelTextureAnimationAdaptor*>&>(textureAnimationAdaptors);
		}

		const std::vector<ModelColorAdaptor*>& getColorAdaptors() const {
			return reinterpret_cast<const std::vector<ModelColorAdaptor*>&>(colorAdaptors);
		}

		const std::vector<ModelTransparencyAdaptor*>& getTransparencyAdaptors() const {
			return reinterpret_cast<const std::vector<ModelTransparencyAdaptor*>&>(transparencyAdaptors);
		}

		const std::vector<ModelBoneAdaptor*>& getBoneAdaptors() const {
			return reinterpret_cast<const std::vector<ModelBoneAdaptor*>&>(boneAdaptors);
		}

		const std::vector<int16_t>& getKeyBoneLookup() const {
			return keyBoneLookup;
		}

		const std::vector<ModelRibbonEmitterAdaptor*>& getRibbonAdaptors() const {
			return reinterpret_cast<const std::vector<ModelRibbonEmitterAdaptor*>&>(ribbonAdaptors);
		}

		const std::vector<ModelParticleEmitterAdaptor*>& getParticleAdaptors() const {
			return reinterpret_cast<const std::vector<ModelParticleEmitterAdaptor*>&>(particleAdaptors);
		}

		const std::vector<ModelAttachmentDefinitionAdaptor*>& getAttachmentDefintionAdaptors() const {
			return reinterpret_cast<const std::vector<ModelAttachmentDefinitionAdaptor*>&>(attachmentDefinitionAdaptors);
		}

		const std::vector<uint32_t>& getGlobalSequences() const {
			return *globalSequences;
		}

		const std::vector<Vector3>& getVertices() const {
			return vertices;
		}

		const std::vector<Vector3>& getNormals() const {
			return normals;
		}

		const std::vector<uint16_t>& getIndices() const {
			return indices;
		}

		const std::vector<Vector2>& getTextureCoords() const {
			return textureCoords;
		}

		const std::vector<Vector3>& getBounds() const {
			return bounds;
		}

		const std::vector<uint16_t>& getBoundTriangles() const {
			return boundTriangles;
		}

		const std::vector<ModelVertexM2>& getRawVertices() const {
			return rawVertices;
		}

		const std::vector<ModelTextureM2>& getTextureDefinitions() const {
			return textureDefinitions;
		}

		const std::vector<uint16_t>& getAttachmentLookups() const {
			return attachmentLookups;
		}


	protected:
		M2Header _header;
		ChunkedFile::Chunks _chunks;

		std::vector<std::unique_ptr<ModelGeosetAdaptor>> geosetAdaptors;
		std::vector<std::unique_ptr<ModelAnimationSequenceAdaptor>> animationSequenceAdaptors;
		std::vector<std::unique_ptr<ModelTextureAnimationAdaptor>> textureAnimationAdaptors;

		std::vector<std::unique_ptr<ModelColorAdaptor>> colorAdaptors;
		std::vector<std::unique_ptr<ModelTransparencyAdaptor>> transparencyAdaptors;

		std::vector<std::unique_ptr<ModelBoneAdaptor>> boneAdaptors;
		std::vector<std::unique_ptr<ModelAttachmentDefinitionAdaptor>> attachmentDefinitionAdaptors;

		std::vector<std::unique_ptr<ModelRibbonEmitterAdaptor>> ribbonAdaptors;
		std::vector<std::unique_ptr<ModelParticleEmitterAdaptor>> particleAdaptors;	

		std::shared_ptr<std::vector<uint32_t>> globalSequences;

		std::vector<Vector3> vertices;
		std::vector<Vector3> normals;
		std::vector<Vector2> textureCoords;
		std::vector<uint16_t> indices;
		std::vector<ModelVertexM2> rawVertices;
		std::vector<Vector3> bounds;
		std::vector<uint16_t> boundTriangles;

		std::vector<ModelTextureM2> textureDefinitions;
		std::vector<uint16_t> attachmentLookups;
		std::vector<int16_t> keyBoneLookup;
		std::vector<uint16_t> animationLookups; 

	private:
		GameFileInfo fileInfo;

		friend class M2Loader;
	};



	//TODO move to better location
	// not part of M2, but used by wmv for rendering.
	struct TextureLoadDef {
		size_t index; 
		ModelTextureM2 defintion;
		GameFileUri uri;
	};

	struct ModelRenderPass {

		ModelRenderPass(
			const ModelRenderFlagsM2& render_flags,
			const ModelTextureUnitM2& texture_unit
		) {
			useTex2 = false;
			useEnvMap = false;
			trans = false;
			texanim = -1; // no texture animation

			blendmode = render_flags.blend;

			unlit = (render_flags.flags & RenderFlags::UNLIT) != 0;
			cull = (render_flags.flags & RenderFlags::TWO_SIDED) == 0;
			billboard = (render_flags.flags & RenderFlags::BILLBOARD) != 0;

			useEnvMap = (texture_unit.textureUnitIndex == -1) && billboard && render_flags.blend > 2;

			noZWrite = (render_flags.flags & RenderFlags::ZBUFFERED) != 0;

			geosetIndex = texture_unit.submeshIndex;
			color = texture_unit.colorIndex;
		}

		ModelRenderPass(ModelRenderPass&&) = default;

		uint32_t indexStart;
		uint32_t indexCount;
		uint32_t vertexStart;
		uint32_t vertexEnd;

		int32_t tex;
		bool useTex2;
		bool useEnvMap;
		bool cull;
		bool trans;
		bool unlit;
		bool noZWrite;
		bool billboard;

		float p;

		int16_t texanim;
		int16_t color;
		int16_t opacity;
		int16_t blendmode;

		int32_t geosetIndex;

		bool swrap;
		bool twrap;

		ColorRGBA<float> ocol;
		ColorRGBA<float> ecol;

		bool operator< (const ModelRenderPass& m) const
		{
			//TODO not sure if still used - remove if not.
			// 
			// This is the old sort order method which I'm pretty sure is wrong - need to try something else.
			// Althogh transparent part should be displayed later, but don't know how to sort it
			// And it will sort by geoset id now.
			return geosetIndex < m.geosetIndex;
		}
	};



	class M2Model : public M2Data {
	public:

		using make_result_t = std::pair<std::unique_ptr<M2Model>, std::vector<TextureLoadDef>>;
		using Factory = std::function<make_result_t(GameFileSystem*, const GameFileUri&)>;

		static make_result_t make(GameFileSystem* fs, const GameFileUri& uri) {
			std::unique_ptr<M2Model> m2 = std::make_unique<M2Model>();

			M2Loader loader(m2.get(), fs, uri);
			m2->modelPathInfo =  ModelPathInfo(m2->getFileInfo().path, fs);	
			m2->renderPasses = std::move(loader.renderPasses);

			return std::make_pair(std::move(m2), std::move(loader.textures));
		}


		const ModelPathInfo& getModelPathInfo() const {
			return modelPathInfo;
		}

		const std::vector<ModelRenderPass>& getRenderPasses() const {
			return renderPasses;
		}

		void updateParticles(size_t animation_index, const AnimationTickArgs& tick) {
			for (auto& particle : particleAdaptors) {
				particle->update(animation_index, tick, reinterpret_cast<std::vector<ModelBoneAdaptor*>&>(boneAdaptors));
			}
		}

		void updateRibbons(size_t animation_index, const AnimationTickArgs& tick) {
			for (auto& ribbon : ribbonAdaptors) {
				ribbon->update(animation_index, tick, reinterpret_cast<std::vector<ModelBoneAdaptor*>&>(boneAdaptors));
			}
		}

		void calculateBones(size_t animation_index, const AnimationTickArgs& tick) {

			const auto keybone_lookup_size = keyBoneLookup.size();
			const auto bone_adaptor_size = boneAdaptors.size();

			if (bone_adaptor_size == 0) {
				return;
			}

			for (auto& bone : boneAdaptors) {
				bone->resetCalculated();
			}

			//TODO check if char different logic?
			if (keybone_lookup_size > KeyBones::BONE_ROOT) {
				const auto keybone_val = keyBoneLookup.at(KeyBones::BONE_ROOT);
				for (auto i = 0; i < keybone_val; i++) {
					boneAdaptors[i]->calculateMatrix(animation_index, tick, reinterpret_cast<std::vector<ModelBoneAdaptor*>&>(boneAdaptors));
				}

				const int32_t upper = std::ranges::min({
					(int32_t)KeyBones::BONE_MAX,
					(int32_t)(keybone_lookup_size - 1),
					(int32_t)(bone_adaptor_size - 1)
					});

				for (int32_t i = KeyBones::BONE_ROOT; i < upper; i++) {
					const auto keybone_val = keyBoneLookup.at(i);
					if (keybone_val >= 0) {
						boneAdaptors[i]->calculateMatrix(animation_index, tick, reinterpret_cast<std::vector<ModelBoneAdaptor*>&>(boneAdaptors));
					}
				}
			}

			for (auto& bone : boneAdaptors) {
				bone->calculateMatrix(animation_index, tick, reinterpret_cast<std::vector<ModelBoneAdaptor*>&>(boneAdaptors));
			}
		}

	protected:
		std::vector<ModelRenderPass> renderPasses;

	private:
		ModelPathInfo modelPathInfo;
	};


}