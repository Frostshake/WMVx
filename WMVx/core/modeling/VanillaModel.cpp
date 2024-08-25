#include "../../stdafx.h"
#include <memory>
#include "VanillaModel.h"
#include "../game/GameConstants.h"
#include "GenericModelAdaptors.h"
#include "../utility/ScopeGuard.h"
#include "../filesystem/MPQFileSystem.h"

namespace core {

	void VanillaModel::load(GameFileSystem* fs, GameFileUri uri, TextureCallback loadTexture)
	{
		RawModel::load(fs, uri, loadTexture);

		std::unique_ptr<MPQFile> file((MPQFile*)fs->openFile(uri).release());

		auto filesize = file->getFileSize();
		if (filesize < sizeof(VanillaModelHeaderM2)) {
			throw BadStructureException(uri.toString().toStdString(), "file size smaller than header");
		}

		auto buffer = std::vector<uint8_t>(filesize);
		file->read(buffer.data(), filesize);
		memcpy(&header, buffer.data(), sizeof(VanillaModelHeaderM2));

		std::string signature((char*)header.id, sizeof(header.id));

		if (signature != "MD20") {
			throw BadSignatureException(uri.toString().toStdString(), signature, "MD20");
		}


		//TODO check header version - should name offset be checked for vanilla?
		//if (header.name.offset != 304 && header.name.offset != 320)
		//{
		//	throw BadStructureException(fileName.toStdString(), "unexpected name offset");
		//}

		// header is now loaded, start loading other data.

		if (header.globalSequences.size) {
			globalSequences->resize(header.globalSequences.size);
			memcpy(globalSequences->data(), buffer.data() + header.globalSequences.offset, sizeof(uint32_t) * header.globalSequences.size);
		}

		auto bonesDefinitions = std::vector<ModelBoneM2<M2_VER_RANGE(M2_VER_VANILLA_MIN, M2_VER_VANILLA_MAX)>>(header.bones.size);
		memcpy(bonesDefinitions.data(), buffer.data() + header.bones.offset, sizeof(ModelBoneM2<M2_VER_RANGE(M2_VER_VANILLA_MIN, M2_VER_VANILLA_MAX)>) * header.bones.size);

		rawVertices.resize(header.vertices.size);
		memcpy(rawVertices.data(), buffer.data() + header.vertices.offset, sizeof(ModelVertexM2) * header.vertices.size);

		vertices.resize(header.vertices.size);
		normals.resize(header.vertices.size);

		for (uint32_t i = 0; i < header.vertices.size; i++) {
			vertices[i] = Vector3::yUpToZUp(rawVertices[i].position);
			normals[i] = Vector3::yUpToZUp(rawVertices[i].normal).normalize();
		}

		bounds.resize(header.boundingVertices.size);
		memcpy(bounds.data(), buffer.data() + header.boundingVertices.offset, sizeof(Vector3) * header.boundingVertices.size);
		for (auto& bound : bounds) {
			bound = Vector3::yUpToZUp(bound);
		}

		boundTriangles.resize(header.boundingTriangles.size);
		memcpy(boundTriangles.data(), buffer.data() + header.boundingTriangles.offset, sizeof(uint16_t) * header.boundingTriangles.size);

		if (header.textures.size) {

			if (header.textures.size > TEXTURE_MAX) {
				throw BadStructureException(uri.toString().toStdString(), "texture size exceeds max");
			}

			textureDefinitions.resize(header.textures.size);
			memcpy(textureDefinitions.data(), buffer.data() + header.textures.offset, sizeof(ModelTextureM2) * header.textures.size);

			auto texdef_index = 0;
			for (const auto& texdef : textureDefinitions) {
				if (texdef.type == (uint32_t)TextureType::FILENAME) {
					QString textureName = QString(std::string((char*)buffer.data() + texdef.name.offset, texdef.name.size).c_str());
					loadTexture(this, texdef_index, texdef, GameFileUri(textureName));
				}
				else {
					loadTexture(this, texdef_index, texdef, (GameFileUri::id_t)0);
				}

				texdef_index++;
			}
		}

		if (header.attachments.size) {
			attachmentDefinitions.resize(header.attachments.size);
			memcpy(attachmentDefinitions.data(), buffer.data() + header.attachments.offset, sizeof(ModelAttachmentM2<M2_VER_RANGE(M2_VER_VANILLA_MIN, M2_VER_VANILLA_MAX)>) * header.attachments.size);

			for (auto& attachDef : attachmentDefinitions) {
				attachmentDefinitionAdaptors.push_back(
					std::make_unique<GenericModelAttachmentDefinitionAdaptor<M2_VER_RANGE(M2_VER_VANILLA_MIN, M2_VER_VANILLA_MAX)>>(std::move(attachDef))
				);
			}
		}


		if (header.attachmentLookup.size) {
			attachmentLookups.resize(header.attachmentLookup.size);
			memcpy(attachmentLookups.data(), buffer.data() + header.attachmentLookup.offset, sizeof(uint16_t) * header.attachmentLookup.size);
		}

		if (header.views.size) {
			//TODO check if multiple views should be loaded.
			// existing implementation looks to always load the zero index view.

			ModelViewM2<M2_VER_RANGE(M2_VER_VANILLA_MIN, M2_VER_VANILLA_MAX)> view;
			memcpy(&view, buffer.data() + header.views.offset, sizeof(ModelViewM2<M2_VER_RANGE(M2_VER_VANILLA_MIN, M2_VER_VANILLA_MAX)>));

			auto indexLookup = std::vector<uint16_t>(view.indices.size);
			auto triangles = std::vector<uint16_t>(view.triangles.size);

			memcpy(indexLookup.data(), buffer.data() + view.indices.offset, sizeof(uint16_t) * view.indices.size);
			memcpy(triangles.data(), buffer.data() + view.triangles.offset, sizeof(uint16_t) * view.triangles.size);
			indices.resize(view.triangles.size);

			for (uint32_t i = 0; i < view.triangles.size; i++) {
				indices[i] = indexLookup[triangles[i]];
			}

			geosets.resize(view.submeshes.size);
			memcpy(geosets.data(), buffer.data() + view.submeshes.offset, sizeof(ModelGeosetM2<M2_VER_RANGE(M2_VER_VANILLA_MIN, M2_VER_VANILLA_MAX)>) * view.submeshes.size);

			for (auto& geoset : geosets) {
				geosetAdaptors.push_back(std::make_unique<GenericOldModelGeosetAdaptor<M2_VER_RANGE(M2_VER_VANILLA_MIN, M2_VER_VANILLA_MAX)>>(&geoset));
			}

			auto modelTextureUnits = std::vector<ModelTextureUnitM2>(view.textureUnits.size);
			memcpy(modelTextureUnits.data(), buffer.data() + view.textureUnits.offset, sizeof(ModelTextureUnitM2) * view.textureUnits.size);


			auto render_flags_source = std::vector<ModelRenderFlagsM2>(header.renderFlags.size);
			memcpy(render_flags_source.data(), buffer.data() + header.renderFlags.offset, sizeof(ModelRenderFlagsM2) * header.renderFlags.size);

			auto textureLookup = std::vector<uint16_t>(header.textureLookup.size);
			memcpy(textureLookup.data(), buffer.data() + header.textureLookup.offset, sizeof(uint16_t) * header.textureLookup.size);

			auto textureAnimLookup = std::vector<uint16_t>(header.uvAnimationLookup.size);
			memcpy(textureAnimLookup.data(), buffer.data() + header.uvAnimationLookup.offset, sizeof(uint16_t) * header.uvAnimationLookup.size);

			auto transparencyLookup = std::vector<uint16_t>(header.transparencyLookup.size);
			memcpy(transparencyLookup.data(), buffer.data() + header.transparencyLookup.offset, sizeof(uint16_t) * header.transparencyLookup.size);

			renderPasses.reserve(view.textureUnits.size);

			for (uint32_t i = 0; i < view.textureUnits.size; i++) {

				const auto& mtu = modelTextureUnits[i];
				const auto& rf = render_flags_source[mtu.renderFlagsIndex];
				ModelRenderPass pass(rf, mtu);
				//TODO TIDY

				pass.indexStart = geosets[pass.geosetIndex].triangleStart;
				pass.indexCount = geosets[pass.geosetIndex].triangleCount;
				pass.vertexStart = geosets[pass.geosetIndex].vertexStart;
				pass.vertexEnd = pass.vertexStart + geosets[pass.geosetIndex].vertexCount;

				pass.tex = textureLookup[mtu.textureId];
				pass.opacity = transparencyLookup[mtu.transparencyIndex];

				pass.trans = pass.blendmode > 0 && pass.opacity > 0;

				pass.p = geosets[pass.geosetIndex].centerMass.z;

				pass.swrap = (textureDefinitions[pass.tex].flags & TextureFlag::WRAPX) != 0;
				pass.twrap = (textureDefinitions[pass.tex].flags & TextureFlag::WRAPY) != 0;

				if ((textureDefinitions[pass.tex].flags & TextureFlag::STATIC) == 0) {
					pass.texanim = textureAnimLookup[mtu.textureAnimationId];
				}

				renderPasses.push_back(std::move(pass));
			}

			//TODO views

		}

		if (header.animations.size) {
			animationSequences.resize(header.animations.size);
			memcpy(animationSequences.data(), buffer.data() + header.animations.offset, sizeof(AnimationSequenceM2<M2_VER_RANGE(M2_VER_VANILLA_MIN, M2_VER_VANILLA_MAX)>) * header.animations.size);

			for (auto& anim_seq : animationSequences) {
				animationSequenceAdaptors.push_back(std::make_unique<GenericModelAnimationSequenceAdaptor<M2_VER_RANGE(M2_VER_VANILLA_MIN, M2_VER_VANILLA_MAX)>>(std::move(anim_seq)));
			}
		}

		if (header.colors.size) {
			auto colourDefinitions = std::vector<ModelColorM2<M2_VER_RANGE(M2_VER_VANILLA_MIN, M2_VER_VANILLA_MAX)>>(header.colors.size);
			memcpy(colourDefinitions.data(), buffer.data() + header.colors.offset, sizeof(ModelColorM2<M2_VER_RANGE(M2_VER_VANILLA_MIN, M2_VER_VANILLA_MAX)>) * header.colors.size);

			colorAdaptors.reserve(colourDefinitions.size());

			for (const auto& colourDef : colourDefinitions) {
				auto color = std::make_unique<ModelColorAdaptorLegacy>();
				auto temp1 = VanillaAnimationBlock<Vector3>::fromDefinition(colourDef.color, buffer);
				auto temp2 = VanillaAnimationBlock<float>::fromDefinition(colourDef.opacity, buffer);

				color->color.init(temp1, globalSequences);
				color->opacity.init(*((VanillaAnimationBlock<int16_t>*)(&temp2)), globalSequences); //TODO TIDY CASTING! -is this of the correct type / is cast needed?

				colorAdaptors.push_back(std::move(color));
			}
		}

		if (header.transparency.size) {

			auto transparencyDefinitions = std::vector<ModelTransparencyM2<M2_VER_RANGE(M2_VER_VANILLA_MIN, M2_VER_VANILLA_MAX)>>(header.transparency.size);
			memcpy(transparencyDefinitions.data(), buffer.data() + header.transparency.offset, sizeof(ModelTransparencyM2<M2_VER_RANGE(M2_VER_VANILLA_MIN, M2_VER_VANILLA_MAX)>) * header.transparency.size);

			transparencyAdaptors.reserve(transparencyDefinitions.size());


			for (const auto& transDef : transparencyDefinitions) {
				auto trans = std::make_unique<ModelTransparencyAdaptorLegacy>();
				auto temp1 = VanillaAnimationBlock<float>::fromDefinition(transDef.transparency, buffer);

				trans->transparency.init(*((VanillaAnimationBlock<int16_t>*)(&temp1)), globalSequences); //TODO TIDY CASTING - is this of the correct type / is cast needed?

				transparencyAdaptors.push_back(std::move(trans));
			}
		}

		if (header.keyBoneLookup.size) {
			keyBoneLookup.resize(header.keyBoneLookup.size);
			memcpy(keyBoneLookup.data(), buffer.data() + header.keyBoneLookup.offset, sizeof(int16_t) * header.keyBoneLookup.size);
		}


		if (bonesDefinitions.size()) {
			boneAdaptors.reserve(bonesDefinitions.size());
			for (const auto& boneDef : bonesDefinitions) {

				auto boneTranslationData = VanillaAnimationBlock<Vector3>::fromDefinition(boneDef.translation, buffer);
				auto boneRotationData = VanillaAnimationBlock<Quaternion>::fromDefinition(boneDef.rotation, buffer);
				auto boneScaleData = VanillaAnimationBlock<Vector3>::fromDefinition(boneDef.scale, buffer);

				auto bone = std::make_unique<ModelBoneAdaptorLegacy>();
				bone->calculated = false;
				bone->boneDefinition = boneDef;

				bone->pivot = Vector3::yUpToZUp(boneDef.pivot);
				bone->billboard = (boneDef.flags & ModelBoneFlags::spherical_billboard) != 0;

				bone->translation.init(boneTranslationData, globalSequences);
				bone->rotation.init(boneRotationData, globalSequences);
				bone->scale.init(boneScaleData, globalSequences);

				bone->translation.fix(Vector3::yUpToZUp);
				bone->rotation.fix([](const Quaternion& q) {
					return Quaternion(-q.x, -q.z, q.y, q.w);
					});
				bone->scale.fix([](const Vector3& v) {
					return Vector3(v.x, v.z, v.y);
					});

				boneAdaptors.push_back(std::move(bone));
			}
		}

		if (header.animationLookup.size) {
			animationLookups.resize(header.animationLookup.size);
			memcpy(animationLookups.data(), buffer.data() + header.animationLookup.offset, sizeof(uint16_t) * header.animationLookup.size);
		}

		if (header.uvAnimations.size) {
			auto texAnimDefs = std::vector<TextureAnimationM2<M2_VER_RANGE(M2_VER_VANILLA_MIN, M2_VER_VANILLA_MAX)>>(header.uvAnimations.size);
			memcpy(texAnimDefs.data(), buffer.data() + header.uvAnimations.offset, sizeof(TextureAnimationM2<M2_VER_RANGE(M2_VER_VANILLA_MIN, M2_VER_VANILLA_MAX)>)* header.uvAnimations.size);

			textureAnimationAdaptors.reserve(texAnimDefs.size());

			for (const auto& texAnimDef : texAnimDefs) {
				// uvAnimations
				assert(false);
			}
		}

		if (header.particleEmitters.size) {
			// particle emitters
		}

		if (header.ribbonEmitters.size) {
			// ribbon emitters
		}

		if (header.events.size) {
			//events
		}

		if (header.cameraLookup.size) {
			// camera?
		}

		if (header.lights.size) {
			//lights
		}
	}

}