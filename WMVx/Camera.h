#pragma once

class Camera {
public:
	Camera() = default;
	Camera(Camera&&) = default;
	virtual ~Camera() {};

	virtual void reset() = 0;
	virtual void setup() = 0;

	virtual void scroll(float change, float factor) = 0;

	virtual void leftMouseStart() = 0;
	virtual void leftMouse(float change_x, float change_y, float factor) = 0;
	virtual void leftMouseEnd() = 0;

	virtual void rightMouseStart() = 0;
	virtual void rightMouse(float change_x, float change_y, float factor) = 0;
	virtual void rightMouseEnd() = 0;
};
