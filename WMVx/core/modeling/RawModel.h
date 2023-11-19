#pragma once

#include <QString>
#include <vector>
#include <memory>
#include <cstdint>

#include "../filesystem/GameFileSystem.h"
#include "../utility/Vector2.h"
#include "../utility/Vector3.h"
#include "../utility/Color.h"
#include "../game/GameConstants.h"
#include "M2Defintions.h"
#include "ModelAdaptors.h"
#include "ModelPathInfo.h"

namespace core {

	struct ModelRenderPass;

	class RawModel {
	public:
		using TextureCallback = std::function<void(const RawModel*, size_t, const ModelTextureM2&, GameFileUri)>;

		RawModel() {
			globalSequences = std::make_shared<std::vector<uint32_t>>();
		}
		RawModel(RawModel&&) = default;
		virtual ~RawModel() {}

		virtual void load(GameFileSystem* fs, GameFileUri uri, TextureCallback loadTexture) {
			fileInfo = fs->asInfo(uri);
			modelPathInfo = ModelPathInfo(fileInfo.path, fs);
		}

		//TODO should be casting?

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

		const std::vector<ModelRenderPass>& getRenderPasses() const {
			return renderPasses;
		}


		void calculateBones(size_t animation_index, const AnimationTickArgs& tick) {

			if (boneAdaptors.size() == 0) {
				return;
			}

			for (auto& bone : boneAdaptors) {
				bone->resetCalculated();
			}

			//TODO check if char different logic?
			if (keyBoneLookup.size() > KeyBones::BONE_ROOT) {
				const auto keybone_val = keyBoneLookup.at(KeyBones::BONE_ROOT);
				for (auto i = 0; i < keybone_val; i++) {
					boneAdaptors[i]->calculateMatrix(animation_index, tick, reinterpret_cast<std::vector<ModelBoneAdaptor*>&>(boneAdaptors));
				}
			}

			const int32_t upper = std::min((int32_t)KeyBones::BONE_MAX, (int32_t)(keyBoneLookup.size() - 1));
			for (int32_t i = KeyBones::BONE_ROOT; i < upper; i++) {
				const auto keybone_val = keyBoneLookup.at(i);
				if (keybone_val >= 0) {
					boneAdaptors[i]->calculateMatrix(animation_index,tick, reinterpret_cast<std::vector<ModelBoneAdaptor*>&>(boneAdaptors));
				}
			}

			for (auto& bone : boneAdaptors) {
				bone->calculateMatrix(animation_index, tick, reinterpret_cast<std::vector<ModelBoneAdaptor*>&>(boneAdaptors));
			}
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

		const GameFileInfo& getFileInfo() const {
			return fileInfo;
		}

		const ModelPathInfo& getModelPathInfo() const {
			return modelPathInfo;
		}

		bool isCharacter() const {
			return modelPathInfo.isCharacter();
		}

		bool isHDCharacter() const {
			return modelPathInfo.isHdCharacter();
		}

	protected:

		std::vector<std::unique_ptr<ModelGeosetAdaptor>> geosetAdaptors;
		std::vector<std::unique_ptr<ModelAnimationSequenceAdaptor>> animationSequenceAdaptors;

		std::vector<std::unique_ptr<ModelTextureAnimationAdaptor>> textureAnimationAdaptors;

		std::vector<std::unique_ptr<ModelColorAdaptor>> colorAdaptors;
		std::vector<std::unique_ptr<ModelTransparencyAdaptor>> transparencyAdaptors;

		std::vector<std::unique_ptr<ModelBoneAdaptor>> boneAdaptors;

		std::vector<std::unique_ptr<ModelRibbonEmitterAdaptor>> ribbonAdaptors;
		std::vector<std::unique_ptr<ModelParticleEmitterAdaptor>> particleAdaptors;

		std::vector<std::unique_ptr<ModelAttachmentDefinitionAdaptor>> attachmentDefinitionAdaptors;

		std::shared_ptr<std::vector<uint32_t>> globalSequences;

		std::vector<Vector3> vertices;
		std::vector<Vector3> normals;
		std::vector<Vector2> textureCoords;
		std::vector<uint16_t> indices;

		std::vector<ModelVertexM2> rawVertices;

		std::vector<Vector3> bounds;
		std::vector<uint16_t> boundTriangles; //TODO CHECK IF CORRECT TYPE

		std::vector<ModelTextureM2> textureDefinitions;

		std::vector<uint16_t> attachmentLookups;

		std::vector<int16_t> keyBoneLookup;

		std::vector<uint16_t> animationLookups; //TODO check correct type, wmv and wiki use different types

		std::vector<ModelRenderPass> renderPasses;

		private:
		GameFileInfo fileInfo;
		ModelPathInfo modelPathInfo;

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

			useEnvMap = (texture_unit.textureUnitIndex == -1) && billboard&& render_flags.blend > 2;

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

};