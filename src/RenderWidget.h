#pragma once

#include <QWidget>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include "core/modeling/Model.h"
#include "core/utility/Color.h"
#include "Camera.h"
#include "WidgetUsesScene.h"
#include <memory>

class RenderWidget : public QOpenGLWidget, protected QOpenGLExtraFunctions, public WidgetUsesScene
{
	Q_OBJECT

	//TODO WHEN OVERRIDING QT METHODS, THE BASE STILL NEEDS TO BE CALLED.

public:
	RenderWidget(QWidget *parent = nullptr);
	~RenderWidget();

public slots:
	void setBackground(core::ColorRGBA<float> color);
	void resetCamera();

protected:
	void initializeGL() override;
	void paintGL() override;
	void resizeGL(int width, int height) override;

	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

	void wheelEvent(QWheelEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

	core::ColorRGBA<float> background;

private:

	std::optional<QPointF> lastMousePosition;
	std::unique_ptr<Camera> camera;

	void renderGrid();
	void renderBounds(const core::Model* model);
	void renderBones(const core::Model* model);
	void renderParticles(const core::ModelTextureInfo* model_texture, const core::M2Model* raw_model);

	inline float inputScaleFactor();

};
