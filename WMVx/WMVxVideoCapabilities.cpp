#include "stdafx.h"
#include "WMVxVideoCapabilities.h"
#include <QtOpenGLWidgets/QOpenGLWidget>
#include "core/utility/Logger.h"

WMVxVideoCapabilities* VideoCapabilities::Singleton::_instance = nullptr;

WMVxVideoCapabilities::WMVxVideoCapabilities(QObject *parent)
	: QObject(parent)
{
	loaded = false;
	openglWidget = nullptr;
}

WMVxVideoCapabilities::~WMVxVideoCapabilities()
{}

bool WMVxVideoCapabilities::load(QOpenGLWidget* widget)
{
	openglWidget = widget;

	hardware.vendor = QString::fromStdString(std::string((char*)glGetString(GL_VENDOR)));
	hardware.version = QString::fromStdString(std::string((char*)glGetString(GL_VERSION)));
	hardware.renderer = QString::fromStdString(std::string((char*)glGetString(GL_RENDERER)));


	auto glewErr = glewInit();

	if (glewErr != GLEW_OK) {
		loaded = false;
		return false;
	}

	glGetIntegerv(GL_MAJOR_VERSION, &support.versionMajor);
	glGetIntegerv(GL_MINOR_VERSION, &support.versionMinor);

	support.version = QString((char*)glGetString(GL_VERSION));

	if (glewIsSupported("GL_EXT_texture_filter_anisotropic"))
		glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &support.anisoFilterLevel);
	else
		support.anisoFilterLevel = 0;

	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &support.maxTextureUnits);

	if (support.versionMajor >= 2) {
		support.nonPowerOfTwo = true;
	}
	else {
		support.nonPowerOfTwo = glewIsSupported("GL_ARB_texture_non_power_of_two") == GL_TRUE;
	}

	support.fragmentProgram = glewIsSupported("GL_ARB_fragment_program") == GL_TRUE;
	support.vertexProgram = glewIsSupported("GL_ARB_vertex_program") == GL_TRUE;
	support.glsl = glewIsSupported("GL_ARB_shading_language_100") == GL_TRUE;
	support.multiTexture = glewIsSupported("GL_ARB_multitexture") == GL_TRUE;
	support.drawRangeElements = glewIsSupported("GL_EXT_draw_range_elements") == GL_TRUE;
	support.vertexBufferObject = glewIsSupported("GL_ARB_vertex_buffer_object") == GL_TRUE;
	support.compression = glewIsSupported("GL_ARB_texture_compression GL_ARB_texture_cube_map GL_EXT_texture_compression_s3tc") == GL_TRUE;
	support.pointSprite = glewIsSupported("GL_ARB_point_sprite GL_ARB_point_parameters") == GL_TRUE;
	support.multiSample = wglewIsSupported("WGL_ARB_multisample") == GL_TRUE;
	support.pixelFormat = wglewIsSupported("WGL_ARB_pixel_format") == GL_TRUE;
	support.frameBufferObject = glewIsSupported("GL_EXT_framebuffer_object") == GL_TRUE;
	support.textureRectangle = glewIsSupported("GL_ARB_texture_rectangle") == GL_TRUE;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &support.maxTextureSize);

	if (support.textureRectangle) {
		glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB, &support.maxTextureSizeRectangle);
	}
	else {
		support.maxTextureSizeRectangle = 0;
	}

	currentMode.colorBits = widget->format().redBufferSize() +  widget->format().greenBufferSize() + widget->format().blueBufferSize();
	currentMode.depthBits = widget->format().depthBufferSize();
	currentMode.alphaBits = widget->format().alphaBufferSize();
	currentMode.antiAliasingSamples = widget->format().samples();
	currentMode.doubleBuffer = widget->format().swapBehavior() == QSurfaceFormat::DoubleBuffer;
	currentMode.acceleration = "test"; // widget->format()
	currentMode.stencilBits = widget->format().stencilBufferSize();

	loaded = true;

	//TODO need to set current mode & determine if compatible
}

std::vector<WMVxVideoCapabilities::DisplayMode> WMVxVideoCapabilities::availableModes() const
{

#ifndef _WINDOWS
#error unsupported platform
#endif

	auto modes = std::vector<DisplayMode>();

	assert(openglWidget != nullptr);

	HDC hDC = GetDC((HWND)openglWidget->winId());

	if (hDC == 0) {
		core::Log::message("Unable to get opengl widget hDC.");
		return modes;
	}

	size_t count_pixel_formats = 0;
	std::vector<GLint> attributes;
	std::vector<GLint> results;

	attributes.push_back(WGL_NUMBER_PIXEL_FORMATS_ARB);
	results.resize(2);
	wglGetPixelFormatAttribivARB(hDC, 0, 0, 1, &attributes[0], &results[0]);	// get the number of contexts we can create
	count_pixel_formats = results[0];

	attributes.clear();
	results.clear();

	if (count_pixel_formats == 0) {
		core::Log::message("Unable to get opengl widget pixel formats.");
		return modes;
	}

	attributes.push_back(WGL_DRAW_TO_WINDOW_ARB);
	attributes.push_back(WGL_STENCIL_BITS_ARB);
	attributes.push_back(WGL_ACCELERATION_ARB);
	attributes.push_back(WGL_DEPTH_BITS_ARB);
	attributes.push_back(WGL_SUPPORT_OPENGL_ARB);
	attributes.push_back(WGL_DOUBLE_BUFFER_ARB);
	attributes.push_back(WGL_PIXEL_TYPE_ARB);
	attributes.push_back(WGL_COLOR_BITS_ARB);
	attributes.push_back(WGL_ALPHA_BITS_ARB);
	attributes.push_back(WGL_ACCUM_BITS_ARB);

	if (glewIsSupported("GL_ARB_multisample")) {
		attributes.push_back(WGL_SAMPLE_BUFFERS_ARB);
		attributes.push_back(WGL_SAMPLES_ARB);
	}


	results.resize(attributes.size());

	for (size_t i = 0; i < count_pixel_formats; i++) {
		if (!wglGetPixelFormatAttribivARB(hDC, (int)i + 1, 0, (UINT)attributes.size(), &attributes[0], &results[0]))
			continue;

		// once we make it here we can look at the pixel data to make sure this is a context we want to use
		// results[0] is WGL_DRAW_TO_WINDOW,  since we only work in windowed mode,  no need to return results for fullscreen mode
		// results[4] is WGL_SUPPORT_OPENGL,  obvious if the mode doesn't support opengl then its useless to us.
		// results[3] is bitdepth,  if it has a 0 bitdepth then its useless to us
		if (results[0] == GL_TRUE && results[4] == GL_TRUE && results[3] > 0) {
			DisplayMode mode;

			if (glewIsSupported("GL_ARB_multisample")) {
				mode.antiAliasingSamples = results[11];
			}
			else {
				mode.antiAliasingSamples = 0;
			}

			mode.stencilBits = results[1];

			mode.depthBits = results[3];
			mode.colorBits = results[7];
			mode.alphaBits = results[8];

			mode.doubleBuffer = results[5] == GL_TRUE;

			if (results[2] == WGL_FULL_ACCELERATION_ARB) {
				mode.acceleration = "Hardware";
			}
			else if (results[2] == WGL_GENERIC_ACCELERATION_ARB) {
				mode.acceleration = "Emulation";
			}
			else {
				mode.acceleration = "Software";
			}

			modes.push_back(mode);
		}
	}

	return modes;
}
