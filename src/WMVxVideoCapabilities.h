#pragma once

#include "core/utility/Singleton.h"
#include <QObject>

class QOpenGLWidget;

class WMVxVideoCapabilities  : public QObject
{
	Q_OBJECT

public:
	WMVxVideoCapabilities(QObject *parent);
	~WMVxVideoCapabilities();

	struct Hardware {
		QString vendor;
		QString version;
		QString renderer;
	};

	struct OpenGLSupport {
		GLint versionMajor;
		GLint versionMinor;
		QString version;
		GLint anisoFilterLevel;
		GLint maxTextureUnits;
		bool nonPowerOfTwo;
		bool fragmentProgram;
		bool vertexProgram;
		bool glsl;
		bool multiTexture;
		bool drawRangeElements;
		bool vertexBufferObject;
		bool compression;
		bool pointSprite;
		bool multiSample;
		bool pixelFormat;
		bool frameBufferObject;
		bool textureRectangle;
		GLint maxTextureSize;
		GLint maxTextureSizeRectangle;
	};

	struct DisplayMode {
		GLint colorBits;
		GLint depthBits;
		GLint alphaBits;
		GLint antiAliasingSamples;
		bool doubleBuffer;
		QString acceleration;
		GLint stencilBits;
	};

	bool load(QOpenGLWidget* widget);

	std::vector<DisplayMode> availableModes() const;

	constexpr bool isLoaded() const {
		return loaded;
	}

	const Hardware& getHardware() const {
		return hardware;
	}

	const DisplayMode& getDisplayMode() const {
		return currentMode;
	}

	const OpenGLSupport& getSupport() const {
		return support;
	}

private:

	Hardware hardware;
	DisplayMode currentMode;
	OpenGLSupport support;
	bool loaded;

	QOpenGLWidget* openglWidget;
};


class VideoCapabilities : public core::Singleton<WMVxVideoCapabilities> {
public:

	static bool isLoaded() {
		assert(_instance != nullptr);
		return _instance->isLoaded();
	}

	static WMVxVideoCapabilities::Hardware hardware() {
		assert(_instance != nullptr);
		return _instance->getHardware();
	}

	static WMVxVideoCapabilities::OpenGLSupport support() {
		assert(_instance != nullptr);
		return _instance->getSupport();
	}

	static std::vector<WMVxVideoCapabilities::DisplayMode> availableModes() {
		assert(_instance != nullptr);
		return _instance->availableModes();
	}

};