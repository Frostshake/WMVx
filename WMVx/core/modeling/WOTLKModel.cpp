#include "../../stdafx.h"
#include "WOTLKModel.h"
#include "../utility/Exceptions.h"
#include "GenericModelAdaptors.h"
#include "WOTLKModelAdaptors.h"
#include "../utility/ScopeGuard.h"
#include <memory>
#include <map>

namespace core {

	void WOTLKModel::load(GameFileSystem* fs, GameFileUri uri, TextureCallback loadTexture)
	{
		RawModel::load(fs, uri, loadTexture);

		MPQFile* file = (MPQFile*)fs->openFile(uri);
		auto animFiles = std::map<size_t, ArchiveFile*>();	//archive files keyed by animation_id

		auto file_guard = sg::make_scope_guard([&]() {
			for (auto& file : animFiles) {
				fs->closeFile(file.second);
			}

			fs->closeFile(file);
		});


		auto filesize = file->getFileSize();
		if (filesize < sizeof(WOTLKModelHeaderM2)) {
			throw BadStructureException(uri.toString().toStdString(), "file size smaller than header");
		}

		auto buffer = std::vector<uint8_t>(filesize);
		file->read(buffer.data(), filesize);
		memcpy(&header, buffer.data(), sizeof(WOTLKModelHeaderM2));

		std::string signature((char*)header.id, sizeof(header.id));

		if (signature != "MD20") {
			throw BadSignatureException(uri.toString().toStdString(), signature, "MD20");
		}

		if (header.name.offset != 304 && header.name.offset != 320)
		{
			throw BadStructureException(uri.toString().toStdString(), "unexpected name offset");
		}

		//TODO check header version.

		// header is now loaded, start loading other data.

		if (header.globalSequences.size) {
			globalSequences->resize(header.globalSequences.size);
			memcpy(globalSequences->data(), buffer.data() + header.globalSequences.offset, sizeof(uint32_t) * header.globalSequences.size);
		}

		auto bonesDefinitions = std::vector<WOTLKModelBoneM2>(header.bones.size);
		memcpy(bonesDefinitions.data(), buffer.data() + header.bones.offset, sizeof(WOTLKModelBoneM2) * header.bones.size);

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
			memcpy(attachmentDefinitions.data(), buffer.data() + header.attachments.offset, sizeof(WOTLKModelAttachmentM2) * header.attachments.size);
			
			for (auto& attachDef : attachmentDefinitions) {
				attachmentDefinitionAdaptors.push_back(
					std::make_unique<WOTLKModelAttachmentDefinitionAdaptor>(&attachDef)
				);
			}
		}

		if (header.attachmentLookup.size) {
			attachmentLookups.resize(header.attachmentLookup.size);
			memcpy(attachmentLookups.data(), buffer.data() + header.attachmentLookup.offset, sizeof(uint16_t) * header.attachmentLookup.size);
		}

		if (header.views) {
			// existing implementation looks to always load the zero index view - best quality lod?
			int view_lod_index = 0;
			QString skinName = getFileInfo().path;
			//remove .m2
			skinName = GameFileUri::removeExtension(skinName) + "00" + ".skin";	

			ArchiveFile* skinFile = fs->openFile(skinName);
			auto skinSize = skinFile->getFileSize();
			auto skinBuffer = std::vector<uint8_t>(skinSize);
			skinFile->read(skinBuffer.data(), skinSize);
			fs->closeFile(skinFile);

			WOTLKModelViewM2 view;
			memcpy(&view, skinBuffer.data(), sizeof(WOTLKModelViewM2));

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

			geosets.resize(view.submeshes.size);
			memcpy(geosets.data(), skinBuffer.data() + view.submeshes.offset, sizeof(WOTLKModelGeosetM2) * view.submeshes.size);

			for (auto& geoset : geosets) {
				geosetAdaptors.push_back(std::make_unique<WOTLKModelGeosetAdaptor>(&geoset));
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

				renderPasses.push_back(pass);
			}
		}

		//indiciesToVerts needed?

		if (header.animations.size) {
			animationSequences.resize(header.animations.size);
			memcpy(animationSequences.data(), buffer.data() + header.animations.offset, sizeof(WOTLKAnimationSequenceM2) * header.animations.size);

			for (auto& anim_seq : animationSequences) {
				animationSequenceAdaptors.push_back(std::make_unique<WOTLKModelAnimationSequenceAdaptor>(&anim_seq));
			}

			for (auto anim_index = 0; anim_index < animationSequences.size(); anim_index++) {

				auto mainAnimId = animationSequences[anim_index].id;
				auto subAnimId = animationSequences[anim_index].variationId;

				const QString& fileName = getFileInfo().path;
				QString animName = fileName.mid(0, fileName.lastIndexOf('.')) + QString("%1-%2.anim").arg(QString::number(mainAnimId), 4, '0').arg(QString::number(subAnimId), 2, '0');

				auto animFile = fs->openFile(animName);
				if (animFile != nullptr) {
					animFiles[anim_index] = animFile;
				}
			}
		}


		if (header.colors.size) {
			auto colourDefinitions = std::vector<WOTLKModelColorM2>(header.colors.size);
			memcpy(colourDefinitions.data(), buffer.data() + header.colors.offset, sizeof(WOTLKModelColorM2) * header.colors.size);

			colorAdaptors.reserve(colourDefinitions.size());

			for (const auto& colourDef : colourDefinitions) {
				auto color = std::make_unique<WOTLKModelColor>();
				auto temp1 = WOTLKAnimationBlock<Vector3>::fromDefinition(colourDef.color, buffer, animFiles);
				auto temp2 = WOTLKAnimationBlock<float>::fromDefinition(colourDef.opacity, buffer, animFiles);

				color->color.init(temp1, globalSequences);
				color->opacity.init(*((WOTLKAnimationBlock<int16_t>*)(&temp2)), globalSequences); //TODO TIDY CASTING!	- is this of the correct type / is cast needed?

				colorAdaptors.push_back(std::move(color));
			}

		}

		if (header.transparency.size) {
			auto transparencyDefinitions = std::vector<WOTLKModelTransparencyM2>(header.transparency.size);
			memcpy(transparencyDefinitions.data(), buffer.data() + header.transparency.offset, sizeof(WOTLKModelTransparencyM2) * header.transparency.size);

			transparencyAdaptors.reserve(transparencyDefinitions.size());

			for (const auto& transDef : transparencyDefinitions) {
				auto trans = std::make_unique<WOTLKModelTransparency>();
				auto temp1 = WOTLKAnimationBlock<float>::fromDefinition(transDef.transparency, buffer, animFiles);

				trans->transparency.init(*((WOTLKAnimationBlock<int16_t>*)(&temp1)), globalSequences); //TODO TIDY CASTING!	- is this of the correct type / is cast needed?

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

				auto boneTranslationData = WOTLKAnimationBlock<Vector3>::fromDefinition(boneDef.translation, buffer, animFiles);
				auto boneRotationData = WOTLKAnimationBlock<Quaternion>::fromDefinition(boneDef.rotation, buffer, animFiles);
				auto boneScaleData = WOTLKAnimationBlock<Vector3>::fromDefinition(boneDef.scale, buffer, animFiles);

				auto bone = std::make_unique<WOTLKBone>();
				bone->calculated = false;
				bone->boneDefinition = boneDef;

				bone->pivot = Vector3::yUpToZUp(boneDef.pivot);
				bone->billboard = (boneDef.flags & ModelBoneFlags::spherical_billboard) != 0;

				bone->translation.init(boneTranslationData, globalSequences);
				bone->rotation.init(*((WOTLKAnimationBlock<PACK_QUATERNION>*)(&boneRotationData)), globalSequences);	//TODO TIDY CASTING - is this of the correct type / is cast needed?
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

		if (header.animationLookup.size) {
			animationLookups.resize(header.animationLookup.size);
			memcpy(animationLookups.data(), buffer.data() + header.animationLookup.offset, sizeof(uint16_t) * header.animationLookup.size);
		}

		if (header.uvAnimations.size) {
			auto texAnimDefs = std::vector<WOTLKTextureAnimationM2>(header.uvAnimations.size);
			memcpy(texAnimDefs.data(), buffer.data() + header.uvAnimations.offset, sizeof(WOTLKTextureAnimationM2) * header.uvAnimations.size);

			textureAnimationAdaptors.reserve(texAnimDefs.size());

			for (const auto& texAnimDef : texAnimDefs) {
				auto texAnim = std::make_unique<WOTLKModelTextureAnimationAdaptor>();

				auto translation = WOTLKAnimationBlock<Vector3>::fromDefinition(texAnimDef.translation, buffer, animFiles);
				auto rotation = WOTLKAnimationBlock<Vector3>::fromDefinition(texAnimDef.rotation, buffer, animFiles);
				auto scale = WOTLKAnimationBlock<Vector3>::fromDefinition(texAnimDef.scale, buffer, animFiles);

				texAnim->translation.init(translation, globalSequences);
				texAnim->rotation.init(rotation, globalSequences);
				texAnim->scale.init(scale, globalSequences);

				textureAnimationAdaptors.push_back(std::move(texAnim));
			}
		}

		if (header.particleEmitters.size) {
			auto particleDefinitons = std::vector<WOTLKModelParticleEmitterM2>(header.particleEmitters.size);
			memcpy(particleDefinitons.data(), buffer.data() + header.particleEmitters.offset, sizeof(WOTLKModelParticleEmitterM2) * header.particleEmitters.size);

			for (const auto& particleDef : particleDefinitons) {

				auto speed = WOTLKAnimationBlock<float>::fromDefinition(particleDef.emissionSpeed, buffer, animFiles);
				auto variation = WOTLKAnimationBlock<float>::fromDefinition(particleDef.speedVariation, buffer, animFiles);
				auto spread = WOTLKAnimationBlock<float>::fromDefinition(particleDef.verticalRange, buffer, animFiles);
				auto lat = WOTLKAnimationBlock<float>::fromDefinition(particleDef.horizontalRange, buffer, animFiles);
				auto gravity = WOTLKAnimationBlock<float>::fromDefinition(particleDef.gravity, buffer, animFiles);
				auto lifespan = WOTLKAnimationBlock<float>::fromDefinition(particleDef.lifespan, buffer, animFiles);
				auto rate = WOTLKAnimationBlock<float>::fromDefinition(particleDef.emissionRate, buffer, animFiles);
				auto areal = WOTLKAnimationBlock<float>::fromDefinition(particleDef.emissionAreaLength, buffer, animFiles);
				auto areaw = WOTLKAnimationBlock<float>::fromDefinition(particleDef.emissionAreaWidth, buffer, animFiles);
				auto deacceleration = WOTLKAnimationBlock<float>::fromDefinition(particleDef.zSource, buffer, animFiles);
				auto enabled = WOTLKAnimationBlock<float>::fromDefinition(particleDef.enabledIn, buffer, animFiles);

				auto particle = std::make_unique<WOTLKModelParticleEmitter>();
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
					const auto order = particle->definition.particleType > 0 ? -1 : 0;
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
			auto ribbonDefintions = std::vector<WOTLKModelRibbonEmitterM2>(header.ribbonEmitters.size);
			memcpy(ribbonDefintions.data(), buffer.data() + header.ribbonEmitters.offset, sizeof(WOTLKModelRibbonEmitterM2) * header.ribbonEmitters.size);

			for (const auto& ribbonDef : ribbonDefintions) {

				auto color = WOTLKAnimationBlock<Vector3>::fromDefinition(ribbonDef.color, buffer, animFiles);
				auto opacity = WOTLKAnimationBlock<float>::fromDefinition(ribbonDef.alpha, buffer, animFiles);
				auto above = WOTLKAnimationBlock<float>::fromDefinition(ribbonDef.heightAbove, buffer, animFiles);
				auto below = WOTLKAnimationBlock<float>::fromDefinition(ribbonDef.heightBelow, buffer, animFiles);

				auto ribbon = std::make_unique<WOTLKModelRibbonEmitter>();
				ribbon->definition = ribbonDef;

				ribbon->color.init(color, globalSequences);
				ribbon->opacity.init(*((WOTLKAnimationBlock<int16_t>*)(&opacity)), globalSequences); //TODO TIDY CASTING!	- is this of the correct type / is cast needed?
				ribbon->above.init(above, globalSequences);
				ribbon->below.init(below, globalSequences);

				ribbon->textures.resize(ribbonDef.textures.size);
				memcpy(ribbon->textures.data(), buffer.data() + ribbonDef.textures.offset, sizeof(uint16_t)* ribbonDef.textures.size);

				ribbon->pos = Vector3::yUpToZUp(ribbonDef.position);
				ribbon->tpos = Vector3::yUpToZUp(ribbonDef.position);

				ribbon->numberOfSegments = (uint32_t)ribbonDef.edgesPerSecond;
				ribbon->length = ribbonDef.edgesPerSecond * ribbonDef.edgeLifetime;

				auto segment = WOTLKModelRibbonEmitter::RibbonSegment();
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
			auto lightDefinitions = std::vector<WOTLKModelLightM2>(header.lights.size);
			memcpy(lightDefinitions.data(), buffer.data() + header.lights.offset, sizeof(WOTLKModelLightM2) * header.lights.size);
			//lights 
		}
	}

};