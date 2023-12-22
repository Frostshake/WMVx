#include "stdafx.h"
#include "RenderWidget.h"
#include "ModelRenderPassRenderer.h"
#include "WMVxVideoCapabilities.h"
#include "BasicCamera.h"
#include "ArcBallCamera.h"
#include "WMVxSettings.h"

RenderWidget::RenderWidget(QWidget* parent)
	: QOpenGLWidget(parent), 
	QOpenGLExtraFunctions(),
	WidgetUsesScene()
{
	camera = std::make_unique<BasicCamera>(BasicCamera());
	//camera = std::make_unique<ArcBallCamera>(ArcBallCamera());

	{
		auto color = Settings::get<QColor>(config::app::background_color);
		background = ColorRGB<float>(color.redF(), color.greenF(), color.blueF());
	}

	//TODO load settings from config
	QSurfaceFormat format;
	format.setRedBufferSize(32);
	format.setGreenBufferSize(32);
	format.setBlueBufferSize(32);
	format.setSamples(32);
	format.setAlphaBufferSize(16);
	format.setRenderableType(QSurfaceFormat::OpenGL);
	setFormat(format);
}

RenderWidget::~RenderWidget()
{}

void RenderWidget::resetCamera()
{
	camera->reset();
}

void RenderWidget::initializeGL()
{
	//TODO connect cleanup

	initializeOpenGLFunctions();

	//TODO set video config from settings. e.g antialising, etc

	if (!VideoCapabilities::instance()->load(this)) {
		Log::message("Unabled to load video capabilities.");
	}
	else {
		Log::message("Video capabilities detected:");
		Log::message(VideoCapabilities::hardware().vendor);
		Log::message(VideoCapabilities::hardware().version);
		Log::message(VideoCapabilities::hardware().renderer);
	}

	//TODO log ogl support

	glClearColor(background.red, background.green, background.blue, 1);

	// ideally we'd be using the delta time between 'timeout' calls for smoother animation, 
	// unfortunatly I've not found any easy/reliable to achieve this with Qt.
	// for simple scene rendering in the app, the ability to change target fps should be enough flexiblity.
	const int updateTick = 1000 / Settings::get<int32_t>(config::rendering::target_fps);
	QTimer* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, [&, updateTick]() {
		update();
		if (scene != nullptr) {
			for (auto& model : scene->models) {
				model->update(updateTick);
			}
		}
	});
	timer->setInterval(updateTick);
	timer->start();
}

void RenderWidget::paintGL()
{
	glClearColor(background.red, background.green, background.blue, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	camera->setup();
	if (scene != nullptr) {

		if (scene->showGrid) {
			renderGrid();
		}

		for (const auto &model : scene->models) {
			const AnimationTickArgs& tick = model->animator.getLastTick();
			glPushMatrix();

			glVertexPointer(3, GL_FLOAT, 0, model->model->getVertices().data());
			glNormalPointer(GL_FLOAT, 0, model->model->getNormals().data());
			glTexCoordPointer(2, GL_FLOAT, 0, model->model->getTextureCoords().data());
			
			if (model->renderOptions.showWireFrame) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}

			glTranslatef(model->renderOptions.position.x, model->renderOptions.position.y, -model->renderOptions.position.z);
			glRotatef(model->renderOptions.rotation.x, 1.0f, 0.0f, 0.0f);
			glRotatef(model->renderOptions.rotation.y, 0.0f, 1.0f, 0.0f);
			glRotatef(model->renderOptions.rotation.z, 0.0f, 0.0f, 1.0f);

			glScalef(model->renderOptions.scale.x, model->renderOptions.scale.y, model->renderOptions.scale.z);
			
			if (model->renderOptions.showRender) {
				glEnable(GL_NORMALIZE);
				for (auto& pass : model->model->getRenderPasses()) {

					// May aswell check that we're going to render the geoset before doing all this crap.
					if (!model->isGeosetIndexVisible(pass.geosetIndex)) {
						continue;
					}

					if (ModelRenderPassRenderer::start(model->renderOptions, model.get(), model->model.get(), model->animator.getAnimationIndex(), pass, tick)) {
						
						glBegin(GL_TRIANGLES);
						for (size_t k = 0, b = pass.indexStart; k < pass.indexCount; k++, b++) {
							uint16_t a = model->model->getIndices()[b];
							glNormal3fv((GLfloat*)&model->animatedNormals[a]);
							glTexCoord2fv((GLfloat*)&model->model->getRawVertices()[a].textureCoords);
							glVertex3fv((GLfloat*)&model->animatedVertices[a]);
						}
						glEnd();

						ModelRenderPassRenderer::finish(pass);
					}
				}

				if (model->renderOptions.showParticles) {
					renderParticles(model.get(), model->model.get());
				}

				glDisable(GL_NORMALIZE);
			}
			
			if (model->renderOptions.showBounds) {
				renderBounds(model.get());
			}

			if (model->renderOptions.showBones) {
				renderBones(model.get());
			}

			if (!model->getAttachments().empty()) {
				glEnable(GL_NORMALIZE);
				for (const auto* attachment : model->getAttachments()) {
					glPushMatrix();

					glVertexPointer(3, GL_FLOAT, 0, attachment->model->getVertices().data());
					glNormalPointer(GL_FLOAT, 0, attachment->model->getNormals().data());
					glTexCoordPointer(2, GL_FLOAT, 0, attachment->model->getTextureCoords().data());

					{
						Matrix m = model->model->getBoneAdaptors()[attachment->bone]->getMat();
						m.transpose();
						glMultMatrixf(m);
						glTranslatef(attachment->position.x, attachment->position.y, attachment->position.z);
					}

					if (model->renderOptions.showRender) {
						for (auto& pass :attachment->model->getRenderPasses()) {
							if (ModelRenderPassRenderer::start(model->renderOptions, attachment, attachment->model.get(), std::nullopt, pass, tick)) {

								glBegin(GL_TRIANGLES);
								for (size_t k = 0, b = pass.indexStart; k < pass.indexCount; k++, b++) {
									uint16_t a = attachment->model->getIndices()[b];
									glNormal3fv((GLfloat*)&attachment->animatedNormals[a]);
									glTexCoord2fv((GLfloat*)&attachment->model->getRawVertices()[a].textureCoords);
									glVertex3fv((GLfloat*)&attachment->animatedVertices[a]);
								}
								glEnd();

								ModelRenderPassRenderer::finish(pass);
							}
						}

						if (model->renderOptions.showParticles) {
							renderParticles(attachment, attachment->model.get());
						}
					}

					if (!attachment->effects.empty()) {
						for (const auto& effect : attachment->effects) {
							glVertexPointer(3, GL_FLOAT, 0, effect->model->getVertices().data());
							glNormalPointer(GL_FLOAT, 0, effect->model->getNormals().data());
							glTexCoordPointer(2, GL_FLOAT, 0, effect->model->getTextureCoords().data());

							{
								Matrix m = model->model->getBoneAdaptors()[attachment->bone]->getMat();
								m.transpose();
								glMultMatrixf(m);
								glTranslatef(attachment->position.x, attachment->position.y, attachment->position.z);
							}

							if (model->renderOptions.showRender) {
								for (auto& pass : effect->model->getRenderPasses()) {
									//TODO not sure what animation index should be used.
									
									if (ModelRenderPassRenderer::start(model->renderOptions, effect.get(), effect->model.get(), std::nullopt, pass, tick)) {

										glBegin(GL_TRIANGLES);
										for (size_t k = 0, b = pass.indexStart; k < pass.indexCount; k++, b++) {
											uint16_t a = effect->model->getIndices()[b];
											glNormal3fv((GLfloat*)&effect->animatedNormals[a]);
											glTexCoord2fv((GLfloat*)&effect->model->getRawVertices()[a].textureCoords);
											glVertex3fv((GLfloat*)&effect->animatedVertices[a]);
										}
										glEnd();

										ModelRenderPassRenderer::finish(pass);
									}
								}

								if (model->renderOptions.showParticles) {
									renderParticles(effect.get(), effect->model.get());
								}
							}
						}
					}

					glPopMatrix();
				}

				glDisable(GL_NORMALIZE);
			}

			if (!model->getMerged().empty()) {
				glEnable(GL_NORMALIZE);
				for (const auto* rel : model->getMerged()) {
					glPushMatrix();

					glVertexPointer(3, GL_FLOAT, 0, rel->model->getVertices().data());
					glNormalPointer(GL_FLOAT, 0, rel->model->getNormals().data());
					glTexCoordPointer(2, GL_FLOAT, 0, rel->model->getTextureCoords().data());
				
					if (model->renderOptions.showRender) {

						for (auto& pass : rel->model->getRenderPasses()) {

							if (!rel->isGeosetIndexVisible(pass.geosetIndex)) {
								continue;
							}

							if (ModelRenderPassRenderer::start(model->renderOptions, rel, rel->model.get(), std::nullopt, pass, tick)) {

								glBegin(GL_TRIANGLES);
								for (size_t k = 0, b = pass.indexStart; k < pass.indexCount; k++, b++) {
									uint16_t a = rel->model->getIndices()[b];
									glNormal3fv((GLfloat*)&rel->animatedNormals[a]);
									glTexCoord2fv((GLfloat*)&rel->model->getRawVertices()[a].textureCoords);
									glVertex3fv((GLfloat*)&rel->animatedVertices[a]);
								}
								glEnd();

								ModelRenderPassRenderer::finish(pass);
							}
						}

						if (model->renderOptions.showParticles) {
							renderParticles(rel, rel->model.get());
						}
					}


					glPopMatrix();
				}
				glDisable(GL_NORMALIZE);
			}

			if (model->renderOptions.showWireFrame) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}

			GLenum err = glGetError();
			assert(err == GL_NO_ERROR);

			glPopMatrix();
		}
	}
}

void RenderWidget::resizeGL(int width, int height)
{
	if (height == 0)										// Prevent A Divide By Zero By
		height = 1;										// Making Height Equal One

	glViewport(0, 0, width, height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f, (float)width / (float)height, 0.1f, 128.0f * 5);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

void RenderWidget::wheelEvent(QWheelEvent* event)
{
	auto delta = static_cast<float>(event->angleDelta().y());
	auto value = delta / 120.f;

	camera->zoom(0.f - (value * inputScaleFactor()));
}

void RenderWidget::mouseMoveEvent(QMouseEvent* event)
{
	//TODO something wierd with camera coords (possibly whole system)

	if (event->buttons().testAnyFlag(Qt::LeftButton)) {
		if (lastMousePosition.has_value()) {
			auto diff = lastMousePosition.value() - event->position();
			lastMousePosition = event->position();

			camera->rotate(Vector2(diff.x() / 5.f, diff.y() / 5.f) * inputScaleFactor());
		}
	}
	
	if (event->buttons().testAnyFlag(Qt::RightButton)) {
		if (lastMousePosition.has_value()) {
			auto diff = lastMousePosition.value() - event->position();
			lastMousePosition = event->position();

			camera->move(
				Vector3(
					diff.x() / 100,
					0,
					0 - (diff.y() / 100)
				) 
				* inputScaleFactor()
			);
		}
	}
}

void RenderWidget::mousePressEvent(QMouseEvent* event)
{
	lastMousePosition = event->position();

	if (event->buttons().testAnyFlag(Qt::LeftButton)) {
		camera->rotateStart();
	}
	
	if (event->buttons().testAnyFlag(Qt::RightButton)) {
		camera->moveStart();
	}
}

void RenderWidget::mouseReleaseEvent(QMouseEvent* event)
{
	lastMousePosition.reset();

	if (event->buttons().testAnyFlag(Qt::LeftButton)) {
		camera->rotateEnd();
	}
	
	if (event->buttons().testAnyFlag(Qt::RightButton)) {
		camera->moveEnd();
	}
}

void RenderWidget::setBackground(ColorRGB<float> color) {
	background = color;
}

void RenderWidget::renderGrid() {
	int count = 0;
	const float plane = 0;

	const GLfloat white[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	glBegin(GL_QUADS);

	for (float i = -20.0f; i <= 20.0f; i += 1.0f) {
		for (float j = -20.0f; j <= 20.0f; j += 1.0f) {
			if ((count % 2) == 0) {
				//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, black);
				glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, white);
				glColor3f(1.0f, 1.0f, 1.0f);
			}
			else {
				//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, black);	
				glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, black);
				glColor3f(0.2f, 0.2f, 0.2f);
			}

			glNormal3f(0, 1, 0);

			glVertex3f(j, plane, i);
			glVertex3f(j, plane, i + 1);
			glVertex3f(j + 1, plane, i + 1);
			glVertex3f(j + 1, plane, i);
			count++;
		}
	}

	glEnd();
}

void RenderWidget::renderBounds(const Model* model) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_TRIANGLES);
	for (size_t i = 0; i < model->model->getBoundTriangles().size(); i++) {
		size_t v = model->model->getBoundTriangles()[i];
		if (v < model->model->getBounds().size()) {
			glVertex3fv((GLfloat*)&model->model->getBounds()[v]);
		}
		else {
			glVertex3f(0, 0, 0);
		}
	}
	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void RenderWidget::renderBones(const Model* model) {
	glDisable(GL_DEPTH_TEST);
	glBegin(GL_LINES);

	for (const auto& bone : model->model->getBoneAdaptors()) {
		if (bone->getParentBoneId() != -1) {
			const auto& point1 = bone->getTranslationPivot();
			const auto& point2 = model->model->getBoneAdaptors()[bone->getParentBoneId()]->getTranslationPivot();
			glVertex3fv((GLfloat*)&point1);
			glVertex3fv((GLfloat*)&point2);
		}
	}

	glEnd();
	glEnable(GL_DEPTH_TEST);
}

void RenderWidget::renderParticles(const ModelTextureInfo* model_texture, const RawModel* raw_model) {

	glPushMatrix();

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);

	

	for (const auto* particle : raw_model->getParticleAdaptors()) {
		glDisable(GL_LIGHTING);
		switch (particle->getBlendType()) {
		case BM_OPAQUE:
			glDisable(GL_BLEND);
			glDisable(GL_ALPHA_TEST);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case BM_TRANSPARENT:
			glDisable(GL_BLEND);
			glEnable(GL_ALPHA_TEST);
			glBlendFunc(GL_ONE, GL_ZERO);
			break;
		case BM_ALPHA_BLEND:
			glEnable(GL_BLEND);
			glDisable(GL_ALPHA_TEST);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case BM_ADDITIVE:
			glEnable(GL_BLEND);
			glDisable(GL_ALPHA_TEST);
			glBlendFunc(GL_SRC_COLOR, GL_ONE);
			break;
		case BM_ADDITIVE_ALPHA:
			glEnable(GL_BLEND);
			glDisable(GL_ALPHA_TEST);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			break;
		case BM_MODULATE:
			glEnable(GL_BLEND);
			glDisable(GL_ALPHA_TEST);
			glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
			break;
		case BM_MODULATEX2:
			glEnable(GL_BLEND);
			glDisable(GL_ALPHA_TEST);
			glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
			break;
		case BM_BLEND_ADD:
			glEnable(GL_BLEND);
			glDisable(GL_ALPHA_TEST);
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			break;
		default:
			assert(false); //TODO handle
		}


		const auto& particle_textures = particle->getTexture();

		assert(particle_textures.size() <= 3);

		constexpr std::array<GLenum, 3> texture_bindings = {
			GL_TEXTURE0_ARB,
			GL_TEXTURE1_ARB,
			GL_TEXTURE2_ARB
		};

		std::array<GLint, 3> textures = {
			Texture::INVALID_ID,
			Texture::INVALID_ID,
			Texture::INVALID_ID
		};
		
		auto tex_match_index = 0;
		for (auto src_tex : particle_textures) {
			if (model_texture->textures.contains(src_tex)) {
				auto tmp_tex_id = model_texture->textures.at(src_tex)->id;
				textures[tex_match_index] = tmp_tex_id;

				glActiveTextureARB(texture_bindings[tex_match_index]);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, tmp_tex_id);
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);

				if (tex_match_index == 0 && particle_textures.size() > 1) {
					glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 4.0);
					glTexEnvf(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 4.0);
				}

				tex_match_index++;
			}
		}

		assert(textures[0] != Texture::INVALID_ID);

		if (textures[0] == Texture::INVALID_ID) {
			return;
		}

		if (tex_match_index > 1) {
			glActiveTextureARB(GL_TEXTURE0_ARB);
		}


		Vector3 vRight(1, 0, 0);
		Vector3 vUp(0, 1, 0);

		// position stuff
		const float f = 1;//0.707106781f; // sqrt(2)/2
		Vector3 bv0 = Vector3(-f, +f, 0);
		Vector3 bv1 = Vector3(+f, +f, 0);
		Vector3 bv2 = Vector3(+f, -f, 0);
		Vector3 bv3 = Vector3(-f, -f, 0);


		const bool billboard = particle->isBillboard();
		const auto& tiles = particle->getTiles();

		if (billboard) {
			//TODO CHECK BILLBOARD LOGIC!
			float modelview[16];
			glGetFloatv(GL_MODELVIEW_MATRIX, modelview);

			vRight = Vector3(modelview[0], modelview[4], modelview[8]);
			vUp = Vector3(modelview[1], modelview[5], modelview[9]); // Spherical billboarding
			//vUp = Vec3D(0,1,0); // Cylindrical billboarding
		}

		//TODO enum for type
		/*
		 * type:
		 * 0	 "normal" particle
		 * 1	large quad from the particle's origin to its position (used in Moonwell water effects)
		 * 2	seems to be the same as 0 (found some in the Deeprun Tram blinky-lights-sign thing)
		 */


		Vector3 vert1, vert2, vert3, vert4;

		glBegin(GL_QUADS);
		for (auto it = particle->getParticles().begin(); it != particle->getParticles().end(); ++it) {

			if (tiles.size() - 1 < it->tile) { // Alfred, 2009.08.07, error prevent
				break;
			}

			glColor4fv((GLfloat*)&it->color); 

			const float size = it->size;

			Vector3 pos = it->position; //TODO handle tpos


			if (particle->getParticleType() == 0 || particle->getParticleType() > 1) {
				if (billboard)
				{

					vert1 = pos - (vRight + vUp) * size;
					vert2 = pos + (vRight - vUp) * size;
					vert3 = pos + (vRight + vUp) * size;
					vert4 = pos - (vRight - vUp) * size;
				}
				else
				{
					vert1 = pos + it->corners[0] * size;
					vert2 = pos + it->corners[1] * size;
					vert3 = pos + it->corners[2] * size;
					vert4 = pos + it->corners[3] * size;
				}

			} else if (particle->getParticleType() == 1) {
				vert1 = pos + bv0 * size;
				vert2 = pos + bv1 * size;
				vert3 = it->origin + bv1 * size;
				vert4 = it->origin + bv0 * size;
			}


			glMultiTexCoord2fvARB(GL_TEXTURE0_ARB, (GLfloat*)&tiles[it->tile].texCoord[0]);
			if (tex_match_index > 1)
				glMultiTexCoord2fvARB(GL_TEXTURE1_ARB, (GLfloat*)&tiles[it->tile].texCoord[0]);
			if (tex_match_index > 2)
				glMultiTexCoord2fvARB(GL_TEXTURE2_ARB, (GLfloat*)&tiles[it->tile].texCoord[0]);
			glVertex3fv(vert1);

			glMultiTexCoord2fvARB(GL_TEXTURE0_ARB, (GLfloat*)&tiles[it->tile].texCoord[1]);
			if (tex_match_index > 1)
				glMultiTexCoord2fvARB(GL_TEXTURE1_ARB, (GLfloat*)&tiles[it->tile].texCoord[1]);
			if (tex_match_index > 2)
				glMultiTexCoord2fvARB(GL_TEXTURE2_ARB, (GLfloat*)&tiles[it->tile].texCoord[1]);
			glVertex3fv(vert2);

			glMultiTexCoord2fvARB(GL_TEXTURE0_ARB, (GLfloat*)&tiles[it->tile].texCoord[2]);
			if (tex_match_index > 1)
				glMultiTexCoord2fvARB(GL_TEXTURE1_ARB, (GLfloat*)&tiles[it->tile].texCoord[2]);
			if (tex_match_index > 2)
				glMultiTexCoord2fvARB(GL_TEXTURE2_ARB, (GLfloat*)&tiles[it->tile].texCoord[2]);
			glVertex3fv(vert3);

			glMultiTexCoord2fvARB(GL_TEXTURE0_ARB, (GLfloat*)&tiles[it->tile].texCoord[3]);
			if (tex_match_index > 1)
				glMultiTexCoord2fvARB(GL_TEXTURE1_ARB, (GLfloat*)&tiles[it->tile].texCoord[3]);
			if (tex_match_index > 2)
				glMultiTexCoord2fvARB(GL_TEXTURE2_ARB, (GLfloat*)&tiles[it->tile].texCoord[3]);
			glVertex3fv(vert4);

		}
		glEnd();


		for (auto exit_index = 0; exit_index < tex_match_index; exit_index++) {

			glActiveTextureARB(texture_bindings[exit_index]);
			if (exit_index == 0) {
				glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 1.0);
				glTexEnvf(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1.0);
			}
			glDisable(GL_TEXTURE_2D);
		}

		if (tex_match_index > 1) {
			glActiveTextureARB(GL_TEXTURE0_ARB);
		}

		glEnable(GL_LIGHTING);
	}

	

	for (const auto* ribbon : raw_model->getRibbonAdaptors()) {

		Vector4 tcolor = ribbon->getTColor();
		GLint texture = Texture::INVALID_ID;

		const auto textures = ribbon->getTexture();
		if (textures.size() > 0) {
			if (model_texture->textures.contains(textures[0])) {
				texture = model_texture->textures.at(textures[0])->id;
			}
		}

		if (texture != Texture::INVALID_ID) {
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, texture);	
		}

		glEnable(GL_BLEND);
		glDisable(GL_LIGHTING);
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_CULL_FACE);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glColor4fv((GLfloat*)&tcolor);

		glBegin(GL_QUAD_STRIP);
		auto it = ribbon->getSegments().begin();
		float l = 0;
		for (; it != ribbon->getSegments().end(); ++it) {
			float u = l / ribbon->getLength();

			glTexCoord2f(u, 0);
			glVertex3fv(it->position + ribbon->getTAbove() * it->up);
			glTexCoord2f(u, 1);
			glVertex3fv(it->position - ribbon->getTBelow() * it->up);

			l += it->len;
		}

		if (ribbon->getSegments().size() > 1) {
			// last segment...?
			--it;
			glTexCoord2f(1, 0);
			glVertex3fv(it->position + ribbon->getTAbove() * it->up + (it->len / it->len0) * it->back);
			glTexCoord2f(1, 1);
			glVertex3fv(it->position - ribbon->getTBelow() * it->up + (it->len / it->len0) * it->back);
		}
		glEnd();

		glColor4f(1, 1, 1, 1);
		glEnable(GL_LIGHTING);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_TRUE);

	}

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

	glPopAttrib();
	glPopClientAttrib();

	glPopMatrix();
}

inline float RenderWidget::inputScaleFactor()
{
	float factor = 1.f;

	if (QApplication::keyboardModifiers().testAnyFlag(Qt::ShiftModifier)) {
		factor /= 2;
	}
	if (QApplication::keyboardModifiers().testAnyFlag(Qt::ControlModifier)) {
		factor /= 4;
	}

	return factor;
}
