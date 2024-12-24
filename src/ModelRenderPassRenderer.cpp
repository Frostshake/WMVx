#include "stdafx.h"
#include "ModelRenderPassRenderer.h"
#include "core/modeling/M2.h"

using namespace core;


bool ModelRenderPassRenderer::start(const RenderOptions& renderOptions, 
	const ModelTextureInfo* textureInfo, 
	const M2Model* model,
	std::optional<size_t> animation_index,
	const ModelRenderPass& pass,
	const core::AnimationTickArgs& tick)
{
	// COLOUR
	// Get the colour and transparency and check that we should even render
	auto ocol = Vector4(1.0f, 1.0f, 1.0f, renderOptions.opacity);
	auto ecol = Vector4(0.0f, 0.0f, 0.0f, 0.0f);

	//TODO check colour and opacity logic
	if (pass.color != -1) {
		const auto& colors = model->getColorAdaptors();
		if (colors.size() > pass.color && colors[pass.color]->colorUses(0)) {
			Vector3 c;

			c = colors[pass.color]->colorValue(0, tick);

			if (animation_index.has_value()) {
				if (colors[pass.color]->opacityUses(animation_index.value())) {
					ocol.w = colors[pass.color]->opacityValue(animation_index.value(), tick);
				}
			}
			
			// old WMV had this check, not sure why, seems to cause some models to appear back
			// keeping code as it might be useful troubleshooting in the future.
			//if (pass.unlit) {
			//	ocol.x = c.x; ocol.y = c.y; ocol.z = c.z;
			//}
			//else {
			//	ocol.x = ocol.y = ocol.z = 0;
			//}

			ocol.x = c.x; ocol.y = c.y; ocol.z = c.z;

			ecol = Vector4(c, ocol.w);
			glMaterialfv(GL_FRONT, GL_EMISSION, ecol);
		}
	}

	if (pass.opacity != -1) {
		const auto& transparencies = model->getTransparencyAdaptors();
		if (transparencies.size() > pass.opacity && transparencies[pass.opacity]->transparencyUses(0)) {
			ocol.w *= transparencies[pass.opacity]->transparencyValue(0, tick);
		}
	}

	// exit and return false before affecting the opengl render state
	if (!((ocol.w > 0) && (pass.color == -1 || ecol.w > 0))) {
		return false;
	}


	GLuint bindtex = Texture::INVALID_ID;
	if (renderOptions.showTexture) {
		bindtex = textureInfo->getTextureId(pass.tex);
	}

	glBindTexture(GL_TEXTURE_2D, bindtex);
	glEnable(GL_TEXTURE_2D);

	switch (pass.blendmode) {
	case BlendMode::BM_OPAQUE:
		break;
	case BlendMode::BM_TRANSPARENT:
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GEQUAL, 0.7f);
		break;
	case BlendMode::BM_ALPHA_BLEND:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // default blend func
		break;
	case BlendMode::BM_ADDITIVE:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_COLOR, GL_ONE);
		break;
	case BlendMode::BM_ADDITIVE_ALPHA:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	case BlendMode::BM_MODULATE:
		glEnable(GL_BLEND);
		glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
		break;
	case BlendMode::BM_MODULATEX2:
		glEnable(GL_BLEND);
		glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
		break;
	case BlendMode::BM_BLEND_ADD:
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		break;
	default:
		assert(false);
		glEnable(GL_BLEND);
		glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
	}

	if (pass.cull) {
		glEnable(GL_CULL_FACE);
	}

	if (pass.swrap) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	}

	if (pass.twrap) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	if (pass.noZWrite) {
		glDepthMask(GL_FALSE);
	}

	// Environmental mapping, material, and effects
	if (pass.useEnvMap)
	{
		// Turn on the 'reflection' shine, using 18.0f as that is what WoW uses based on the reverse engineering
		// This is now set in InitGL(); - no need to call it every render.
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 18.0f);

		// env mapping
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);

		const GLint maptype = GL_SPHERE_MAP;
		//const GLint maptype = GL_REFLECTION_MAP_ARB;

		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, maptype);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, maptype);
	}

	if (pass.texanim != -1) {
		glMatrixMode(GL_TEXTURE);
		glPushMatrix();

		if (animation_index.has_value()) {
			if (model->getTextureAnimationAdaptors().size() > pass.texanim) {
				const auto* texAnim = model->getTextureAnimationAdaptors().at(pass.texanim);
				glLoadIdentity();

				if (texAnim->translationUses(animation_index.value())) {
					const auto transVal = texAnim->translationValue(animation_index.value(), tick);
					glTranslatef(transVal.x, transVal.y, transVal.z);
				}

				if (texAnim->rotationUses(animation_index.value())) {
					const auto rotVal = texAnim->rotationValue(animation_index.value(), tick);
					//TODO check logic
					//glRotatef(rval.x, 0, 0, 1); // this is wrong, I have no idea what I'm doing here ;)
					glRotatef(rotVal.x, 0, 0, 1);
				}

				if (texAnim->scaleUses(animation_index.value())) {
					const auto scaleVal = texAnim->scaleValue(animation_index.value(), tick);
					glScalef(scaleVal.x, scaleVal.y, scaleVal.z);
				}
			}
		}

	}

	//still having weird issues with ocol causing black eyelids :(
	glColor4fv(ocol);

	if (pass.unlit) {
		glDisable(GL_LIGHTING);
	}

	if (pass.blendmode <= 1 && ocol.w < 1.0f) {
		glEnable(GL_BLEND);
	}


	return true;
}

void ModelRenderPassRenderer::finish(const ModelRenderPass& pass)
{
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (pass.noZWrite) {
		glDepthMask(GL_TRUE);
	}

	if (pass.texanim != -1) {
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}

	if (pass.unlit) {
		glEnable(GL_LIGHTING);
	}

	if (pass.cull) {
		glDisable(GL_CULL_FACE);
	}


	if (pass.useEnvMap) {
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
	}

	if (pass.swrap) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	}

	if (pass.twrap) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	if (pass.opacity != -1 || pass.color != -1) {
		GLfloat cone[4] = { 1.0f,1.0f, 1.0f, 1.0f };
		glMaterialfv(GL_FRONT, GL_EMISSION, cone);

		//auto ocol = result.ocol;
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		//glMaterialfv(GL_FRONT, GL_AMBIENT, ocol);
		//ocol = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		//glMaterialfv(GL_FRONT, GL_DIFFUSE, ocol);
	}
}
