#include "../../stdafx.h"
#include "BFAModel.h"
#include "../filesystem/CascFileSystem.h"
#include "../filesystem/ChunkedFile.h"
#include "../utility/Exceptions.h"
#include "GenericModelAdaptors.h"
#include "BFAModelAdaptors.h"
#include "BFAAnimation.h"
#include "BFAModelSupport.h"
#include "../utility/ScopeGuard.h"

namespace core {
	void BFAModel::load(GameFileSystem* fs, GameFileUri uri, TextureCallback loadTexture)
	{
		RawModel::load(fs, uri, loadTexture);

		CascFile* file = (CascFile*)fs->openFile(uri);
		ChunkedFile chunked;
		chunked.open(file);
		auto animFiles = std::map<size_t, ArchiveFile*>();	//archive files keyed by animation_id

		auto file_guard = sg::make_scope_guard([&]() {
			for (auto& file : animFiles) {
				fs->closeFile(file.second);
			}

			fs->closeFile(file);
		});

		auto buffer = std::vector<uint8_t>();
		auto md21 = chunked.get("MD21");

		if (md21.has_value()) {
			buffer.resize(md21.value().size);
			file->read(buffer.data(), md21.value().size, md21.value().offset);
		}
		else {
			auto filesize = file->getFileSize();
			buffer.resize(filesize);
			file->read(buffer.data(), filesize);
		}

		if (buffer.size() < sizeof(BFAModelHeaderM2)) {
			throw BadStructureException(uri.toString().toStdString(), "file size smaller than header");
		}

		memcpy(&header, buffer.data(), sizeof(BFAModelHeaderM2));
		{
			std::string signature((char*)header.id, sizeof(header.id));

			if (signature != "MD20") {
				throw BadSignatureException(uri.toString().toStdString(), signature, "MD20");
			}
		}

		//TODO all chunks should be accessed by chunk type, not int veriables directly

		//TODO check header version / validate name offset?

		uint32_t skeletonFileId = 0;
		CascFile* skeletonFile = nullptr;
		ChunkedFile skeletonChunked;
		auto skid_chunk = chunked.get("SKID");
		if (skid_chunk.has_value()) {
			assert(sizeof(skeletonFileId) == skid_chunk.value().size);
			file->read(&skeletonFileId, skid_chunk.value().size, skid_chunk.value().offset);
			skeletonFile = (CascFile*)fs->openFile(skeletonFileId);
			if (skeletonFile != nullptr) {
				skeletonChunked.open(skeletonFile);
			}
		}

		if (skid_chunk.has_value()) {
			auto sks1_chunk = skeletonChunked.get("SKS1");
			if (sks1_chunk.has_value()) {
				M2Chunk_SKS1 sks1;
				assert(sizeof(sks1) <= sks1_chunk.value().size);
				skeletonFile->read(&sks1, sks1_chunk.value().size, sks1_chunk.value().offset);

				if (sks1.globalSequences.size) {
					//TODO THIS HASNT BEEN TESTED! - not sure if the chunk offset is meant to be incuded with the offset or not
					globalSequences->resize(sks1.globalSequences.size);
					skeletonFile->read(globalSequences->data(), sizeof(uint32_t) * sks1.globalSequences.size, sks1_chunk.value().offset + sks1.globalSequences.offset);

				}

				//TODO read SKPD
			}

		}
		else if (header.globalSequences.size) {
			globalSequences->resize(header.globalSequences.size);
			memcpy(globalSequences->data(), buffer.data() + header.globalSequences.offset, sizeof(uint32_t) * header.globalSequences.size);
		}


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


			auto txid_chunk = chunked.get("TXID");
			std::vector<M2Chunk_TXID> txids;
			if (txid_chunk.has_value()) {
				txids.resize(txid_chunk.value().size / sizeof(M2Chunk_TXID));
				file->read(txids.data(), txid_chunk.value().size, txid_chunk.value().offset);
			}

			auto texdef_index = 0;
			for (const auto& texdef : textureDefinitions) {
				if (texdef.type == (uint32_t)TextureType::FILENAME) {
					if (txids.size() > 0) {
						loadTexture(this, texdef_index, texdef, GameFileUri(txids[texdef_index].fileDataId));
					}
					else {
						QString textureName = QString(std::string((char*)buffer.data() + texdef.name.offset, texdef.name.size).c_str());
						loadTexture(this, texdef_index, texdef, GameFileUri(textureName));
					}
				}
				else {
					loadTexture(this, texdef_index, texdef, (GameFileUri::id_t)0);
				}
				texdef_index++;
			}
		}


		if (skid_chunk.has_value()) {
			auto ska1_chunk = skeletonChunked.get("SKA1");
			if (ska1_chunk.has_value()) {
				M2Chunk_SKA1 ska1;
				assert(sizeof(ska1) <= ska1_chunk.value().size);
				skeletonFile->read(&ska1, ska1_chunk.value().size, ska1_chunk.value().offset);

				//TODO

				if (ska1.attachments.size) {
					//...
				}

				if (ska1.attachmentLookup.size) {
					//...
				}
			}
		}
		else {
			if (header.attachments.size) {
				attachmentDefinitions.resize(header.attachments.size);
				memcpy(attachmentDefinitions.data(), buffer.data() + header.attachments.offset, sizeof(BFAModelAttachmentM2) * header.attachments.size);
			}

			if (header.attachmentLookup.size) {
				attachmentLookups.resize(header.attachmentLookup.size);
				memcpy(attachmentLookups.data(), buffer.data() + header.attachmentLookup.offset, sizeof(uint16_t) * header.attachmentLookup.size);
			}
		}

		for (auto& attachDef : attachmentDefinitions) {
			attachmentDefinitionAdaptors.push_back(
				std::make_unique<BFAModelAttachmentDefinitionAdaptor>(&attachDef)
			);
		}


		if (header.views) {
			int view_lod_index = 0;

			ArchiveFile* skinFile = nullptr;
			QString skinName;
			auto sfid_chunk = chunked.get("SFID");

			if (sfid_chunk.has_value()) {

				std::vector<uint32_t> skinFileIds(sfid_chunk.value().size / sizeof(uint32_t), 99);
				file->read(skinFileIds.data(), sfid_chunk.value().size, sfid_chunk.value().offset);

				if (skinFileIds.size() > view_lod_index) {
					skinName = QString::number(skinFileIds[view_lod_index]);
					skinFile = fs->openFile(skinFileIds[view_lod_index]);
				}
			}
			else {
				skinName = getFileInfo().path;
				skinName = GameFileUri::removeExtension(skinName) + "00" + ".skin";
				skinFile = fs->openFile(skinName);
			}

			if (skinFile != nullptr) {
				auto skinSize = skinFile->getFileSize();
				auto skinBuffer = std::vector<uint8_t>(skinSize);
				skinFile->read(skinBuffer.data(), skinSize);
				fs->closeFile(skinFile);

				BFAModelViewM2 view;
				memcpy(&view, skinBuffer.data(), sizeof(BFAModelViewM2));

				std::string skin_signature((char*)view.id, sizeof(view.id));
				if (skin_signature != "SKIN") {
					throw BadSignatureException(skinName.toStdString(), skin_signature, "SKIN");
				}

				auto indexLookup = std::vector<uint16_t>(view.indices.size);
				auto triangles = std::vector<uint16_t>(view.triangles.size);
				memcpy(indexLookup.data(), skinBuffer.data() + view.indices.offset, sizeof(uint16_t) * view.indices.size);
				memcpy(triangles.data(), skinBuffer.data() + view.triangles.offset, sizeof(uint16_t) * view.triangles.size);
				indices.resize(view.triangles.size);

				for (uint32_t i = 0; i < view.triangles.size; i++) {
					indices[i] = indexLookup[triangles[i]];
				}

#ifdef _DEBUG
				{
					uint32_t geosets_size = view.textureUnits.offset - view.submeshes.offset;
					const auto geoset_struct_size = sizeof(BFAModelGeosetM2);
					assert((geosets_size / view.submeshes.size) == geoset_struct_size);
				}
#endif

				geosets.resize(view.submeshes.size);
				memcpy(geosets.data(), skinBuffer.data() + view.submeshes.offset, sizeof(BFAModelGeosetM2) * view.submeshes.size);


				// sometimes triangle start can overflow int16, count manually to fix this (unsure if there is a more reliable way within the data?)
				uint32_t custom_triangle_start = 0;
				for (auto& geoset : geosets) {
					geosetAdaptors.push_back(std::make_unique<BFAModelGeosetAdaptor>(&geoset, custom_triangle_start));
					custom_triangle_start += geoset.triangleCount;
				}

				auto modelTextureUnits = std::vector<ModelTextureUnitM2>(view.textureUnits.size);
				memcpy(modelTextureUnits.data(), skinBuffer.data() + view.textureUnits.offset, sizeof(ModelTextureUnitM2) * view.textureUnits.size);

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

					pass.indexStart = geosetAdaptors[pass.geosetIndex]->getTriangleStart();
					pass.indexCount = geosetAdaptors[pass.geosetIndex]->getTriangleCount();
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

					renderPasses.push_back(pass);

				}

				//TODO VIEWS

				//TODO do render passes need special sorting? see wmv source
			}
		}

		if (skid_chunk.has_value()) {

			auto afid_chunk = skeletonChunked.get("AFID");
			if (afid_chunk.has_value()) {
				std::vector<M2Chunk_AFID> afids;
				afids.resize(afid_chunk.value().size / sizeof(M2Chunk_AFID));

				skeletonFile->read(afids.data(), afid_chunk.value().size, afid_chunk.value().offset);

				//TODO animations
				//assert(false);
			}
		}
		else {
			if (header.animations.size) {

				animationSequences.resize(header.animations.size);
				memcpy(animationSequences.data(), buffer.data() + header.animations.offset, sizeof(BFAAnimationSequenceM2) * header.animations.size);

				for (auto& anim_seq : animationSequences) {
					animationSequenceAdaptors.push_back(std::make_unique<BFAModelAnimationSequenceAdaptor>(&anim_seq));
				}

				auto afid_chunk = chunked.get("AFID");

				for (auto anim_index = 0; anim_index < animationSequences.size(); anim_index++) {

					auto mainAnimId = animationSequences[anim_index].id;
					auto subAnimId = animationSequences[anim_index].variationId;
					ArchiveFile* animFile = nullptr;

					
					if (afid_chunk.has_value()) {
						//TODO check afids
						//TEST WITH AZSHARA NAGA
						//assert(false);
					}
					else {
	
						const QString& fileName = getFileInfo().path;
						QString animName = fileName.mid(0, fileName.lastIndexOf('.')) + QString("%1-%2.anim").arg(QString::number(mainAnimId), 4, '0').arg(QString::number(subAnimId), 2, '0');
						animFile = fs->openFile(animName);
					}

					
					if (animFile != nullptr) {
						animFiles[anim_index] = animFile;
					}
				}
			}

			if (header.animationLookup.size) {
				animationLookups.resize(header.animationLookup.size);
				memcpy(animationLookups.data(), buffer.data() + header.animationLookup.offset, sizeof(uint16_t) * header.animationLookup.size);
			}
		}

		if (header.colors.size) {

			auto colourDefinitions = std::vector<BFAModelColorM2>(header.colors.size);
			memcpy(colourDefinitions.data(), buffer.data() + header.colors.offset, sizeof(BFAModelColorM2) * header.colors.size);

			colorAdaptors.reserve(colourDefinitions.size());

			for (const auto& colourDef : colourDefinitions) {
				auto color = std::make_unique<BFAModelColor>();
				auto temp1 = BFAAnimationBlock<Vector3>::fromDefinition(colourDef.color, buffer, animFiles);
				auto temp2 = BFAAnimationBlock<float>::fromDefinition(colourDef.opacity, buffer, animFiles);

				color->color.init(temp1, globalSequences);
				color->opacity.init(*((WOTLKAnimationBlock<int16_t>*)(&temp2)), globalSequences); //TODO TIDY CASTING!	 is this of the correct type / is cast needed?

				colorAdaptors.push_back(std::move(color));
			}
		}

		if (header.transparency.size) {

			auto transparencyDefinitions = std::vector<BFAModelTransparencyM2>(header.transparency.size);
			memcpy(transparencyDefinitions.data(), buffer.data() + header.transparency.offset, sizeof(BFAModelTransparencyM2) * header.transparency.size);

			transparencyAdaptors.reserve(transparencyDefinitions.size());

			for (const auto& transDef : transparencyDefinitions) {
				auto trans = std::make_unique<BFAModelTransparency>();
				auto temp1 = BFAAnimationBlock<float>::fromDefinition(transDef.transparency, buffer, animFiles);

				trans->transparency.init(*((WOTLKAnimationBlock<int16_t>*)(&temp1)), globalSequences); //TODO TIDY CASTING!	is this of the correct type / is cast needed?

				transparencyAdaptors.push_back(std::move(trans));
			}
		}

		if (skid_chunk.has_value()) {
			//TODO bone lookup & bones
			//assert(false);
		}
		else {
			if (header.keyBoneLookup.size) {
				keyBoneLookup.resize(header.keyBoneLookup.size);
				memcpy(keyBoneLookup.data(), buffer.data() + header.keyBoneLookup.offset, sizeof(int16_t) * header.keyBoneLookup.size);
			}

			auto bonesDefinitions = std::vector<BFAModelBoneM2>(header.bones.size);
			memcpy(bonesDefinitions.data(), buffer.data() + header.bones.offset, sizeof(BFAModelBoneM2) * header.bones.size);

			if (bonesDefinitions.size()) {
				boneAdaptors.reserve(bonesDefinitions.size());
				for (const auto& boneDef : bonesDefinitions) {

					auto boneTranslationData = BFAAnimationBlock<Vector3>::fromDefinition(boneDef.translation, buffer, animFiles);
					auto boneRotationData = BFAAnimationBlock<Quaternion>::fromDefinition(boneDef.rotation, buffer, animFiles);
					auto boneScaleData = BFAAnimationBlock<Vector3>::fromDefinition(boneDef.scale, buffer, animFiles);

					auto bone = std::make_unique<BFABone>();
					bone->calculated = false;
					bone->boneDefinition = boneDef;

					bone->pivot = Vector3::yUpToZUp(boneDef.pivot);
					bone->billboard = (boneDef.flags & ModelBoneFlags::spherical_billboard) != 0;
	
					bone->translation.init(boneTranslationData, globalSequences);
					bone->rotation.init(*((WOTLKAnimationBlock<PACK_QUATERNION>*)(&boneRotationData)), globalSequences);	//TODO TIDY CASTING! -  is this of the correct type / is cast needed?
					bone->scale.init(boneScaleData, globalSequences);

					bone->rotation.fixOversize();

					//assert(bone->translation.data->size() == bone->rotation.data->size());
					//assert(bone->rotation.data->size() == bone->scale.data->size());

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
		}

		if (header.uvAnimations.size) {
			auto texAnimDefs = std::vector<BFATextureAnimationM2>(header.uvAnimations.size);
			memcpy(texAnimDefs.data(), buffer.data() + header.uvAnimations.offset, sizeof(BFATextureAnimationM2) * header.uvAnimations.size);

			textureAnimationAdaptors.reserve(texAnimDefs.size());

			for (const auto& texAnimDef : texAnimDefs) {
				auto texAnim = std::make_unique<BFAModelTextureAnimationAdaptor>();

				auto translation = BFAAnimationBlock<Vector3>::fromDefinition(texAnimDef.translation, buffer, animFiles);
				auto rotation = BFAAnimationBlock<Vector3>::fromDefinition(texAnimDef.rotation, buffer, animFiles);
				auto scale = BFAAnimationBlock<Vector3>::fromDefinition(texAnimDef.scale, buffer, animFiles);

				texAnim->translation.init(translation, globalSequences);
				texAnim->rotation.init(rotation, globalSequences);
				texAnim->scale.init(scale, globalSequences);

				textureAnimationAdaptors.push_back(std::move(texAnim));
			}
		}

		if (header.particleEmitters.size) {
			auto particleDefinitons = std::vector<BFAModelParticleEmitterM2>(header.particleEmitters.size);
			memcpy(particleDefinitons.data(), buffer.data() + header.particleEmitters.offset, sizeof(BFAModelParticleEmitterM2)* header.particleEmitters.size);

			for (const auto& particleDef : particleDefinitons) {

				auto speed = BFAAnimationBlock<float>::fromDefinition(particleDef.emissionSpeed, buffer, animFiles);
				auto variation = BFAAnimationBlock<float>::fromDefinition(particleDef.speedVariation, buffer, animFiles);
				auto spread = BFAAnimationBlock<float>::fromDefinition(particleDef.verticalRange, buffer, animFiles);
				auto lat = BFAAnimationBlock<float>::fromDefinition(particleDef.horizontalRange, buffer, animFiles);
				auto gravity = BFAAnimationBlock<float>::fromDefinition(particleDef.gravity, buffer, animFiles);
				auto lifespan = BFAAnimationBlock<float>::fromDefinition(particleDef.lifespan, buffer, animFiles);
				auto rate = BFAAnimationBlock<float>::fromDefinition(particleDef.emissionRate, buffer, animFiles);
				auto areal = BFAAnimationBlock<float>::fromDefinition(particleDef.emissionAreaLength, buffer, animFiles);
				auto areaw = BFAAnimationBlock<float>::fromDefinition(particleDef.emissionAreaWidth, buffer, animFiles);
				auto deacceleration = BFAAnimationBlock<float>::fromDefinition(particleDef.zSource, buffer, animFiles);
				auto enabled = BFAAnimationBlock<float>::fromDefinition(particleDef.enabledIn, buffer, animFiles);

				auto particle = std::make_unique<BFAModelParticleEmitter>();
				particle->definition = particleDef;

				particle->speed.init(speed, globalSequences);
				particle->variation.init(variation, globalSequences);
				particle->spread.init(spread, globalSequences);
				particle->lat.init(lat, globalSequences);
				particle->gravity.init(gravity, globalSequences);
				particle->lifespan.init(lifespan, globalSequences);
				particle->rate.init(rate, globalSequences);
				particle->areal.init(areal, globalSequences);
				particle->areaw.init(areaw, globalSequences);
				particle->deacceleration.init(deacceleration, globalSequences);
				particle->enabled.init(enabled, globalSequences);
				particle->rem = 0;

				{
					//TODO check logic here, wtf is it all
					Vector3 colors2[3];
					memcpy(colors2, buffer.data() + particleDef.params.color.keys.offset, sizeof(Vector3) * 3);
					for (size_t i = 0; i < 3; i++) {
						float opacity = *(short*)(buffer.data() + particleDef.params.opacity.keys.offset + i * 2);
						particle->colors[i] = Vector4(colors2[i].x / 255.0f, colors2[i].y / 255.0f, colors2[i].z / 255.0f, opacity / 32767.0f);
						particle->sizes[i] = (*(float*)(buffer.data() + particleDef.params.scale.keys.offset + i * sizeof(Vector2))) * particleDef.params.burstMultiplier;		//TODO check logic, wtf was mta.p.scales[i] ???
					}
				}

				switch (particleDef.emitterType) {
				case ParticleEmitterType::PLANE:
					particle->generator = &ParticleFactory::plane;
					break;
				case ParticleEmitterType::SPHERE:
					particle->generator = &ParticleFactory::sphere;
					break;
				default:
					assert(false); //TODO handle all types
				}

				auto initTile = [&particle](Vector2* tc, int num) {
					const auto order = 0;
					//const auto order = particle->definition.particleType > 0 ? -1 : 0;
					const auto rows = std::max(1, (int)particle->definition.textureDimensionRows);
					const auto cols = std::max(1, (int)particle->definition.textureDimensionColumns);

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
					WOTLKModelParticleEmitter::TexCoordSet tc;
					initTile(tc.texCoord, (int)i);
					particle->tiles.push_back(tc);
				}

				particle->position = Vector3::yUpToZUp(particleDef.position);

				particleAdaptors.push_back(std::move(particle));

			}
		}

		if (header.ribbonEmitters.size) {

			auto ribbonDefintions = std::vector<BFAModelRibbonEmitterM2>(header.ribbonEmitters.size);
			memcpy(ribbonDefintions.data(), buffer.data() + header.ribbonEmitters.offset, sizeof(BFAModelRibbonEmitterM2)* header.ribbonEmitters.size);

			for (const auto& ribbonDef : ribbonDefintions) {

				auto color = BFAAnimationBlock<Vector3>::fromDefinition(ribbonDef.color, buffer, animFiles);
				auto opacity = BFAAnimationBlock<float>::fromDefinition(ribbonDef.alpha, buffer, animFiles);
				auto above = BFAAnimationBlock<float>::fromDefinition(ribbonDef.heightAbove, buffer, animFiles);
				auto below = BFAAnimationBlock<float>::fromDefinition(ribbonDef.heightBelow, buffer, animFiles);

				auto ribbon = std::make_unique<BFAModelRibbonEmitter>();
				ribbon->definition = ribbonDef;

				ribbon->color.init(color, globalSequences);
				ribbon->opacity.init(*((WOTLKAnimationBlock<int16_t>*)(&opacity)), globalSequences); //TODO TIDY CASTING! - is this of the correct type / is cast needed?
				ribbon->above.init(above, globalSequences);
				ribbon->below.init(below, globalSequences);

				ribbon->textures.resize(ribbonDef.textures.size);
				memcpy(ribbon->textures.data(), buffer.data() + ribbonDef.textures.offset, sizeof(uint16_t)* ribbonDef.textures.size);

				ribbon->pos = Vector3::yUpToZUp(ribbonDef.position);
				ribbon->tpos = Vector3::yUpToZUp(ribbonDef.position);

				ribbon->numberOfSegments = (uint32_t)ribbonDef.edgesPerSecond;
				ribbon->length = ribbonDef.edgesPerSecond * ribbonDef.edgeLifetime;

				auto segment = BFAModelRibbonEmitter::RibbonSegment();
				segment.position = ribbon->tpos;
				segment.len = 0;
				ribbon->segments.push_back(segment);

				ribbonAdaptors.push_back(std::move(ribbon));
			}
		}


		if (header.events.size) {
			//events
		}

		if (header.cameras.size) {
			//cameras
		}

		if (header.lights.size) {
			//lights
		}

	}

}