#pragma once

#include "core/utility/Vector3.h"
#include "core/utility/Vector2.h"

using namespace core;

class Camera {
public:
	Camera() = default;
	Camera(Camera&&) = default;
	virtual ~Camera() {};

	virtual void reset() = 0;
	virtual void setup() = 0;

	virtual void zoom(float zoom) = 0;

	virtual void rotateStart() = 0;
	virtual void rotate(Vector2 angle_offset) = 0;
	virtual void rotateEnd() = 0;

	virtual void moveStart() = 0;
	virtual void move(Vector3 position_offset) = 0;
	virtual void moveEnd() = 0;
};
