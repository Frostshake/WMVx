#pragma once

#include "Camera.h"
#include "core/utility/Vector2.h"
#include "core/utility/Vector3.h"

class BasicCamera : public Camera
{
public:
	BasicCamera();
	BasicCamera(BasicCamera&&) = default;
	virtual ~BasicCamera() {};

	static constexpr const char* identifier = "basic";

	virtual void reset() override;
	virtual void setup() override;

	virtual void key(float change_x, float change_y, bool alternative, float factor) override;

	virtual void leftMouseStart() override;
	virtual void leftMouse(float change_x, float change_y, float factor) override;
	virtual void leftMouseEnd() override;

	virtual void rightMouseStart() override;
	virtual void rightMouse(float change_x, float change_y, float factor) override;
	virtual void rightMouseEnd() override;

	virtual void scroll(float change, float factor) override;

protected:

	void rotateX(float angle);
	void rotateY(float angle);
	void rotateZ(float angle);

	void moveForward(float distance);
	void moveUpward(float distance);
	void strafe(float distance);

	core::Vector3 viewDirection;
	core::Vector3 rightVector;
	core::Vector3 upVector;
	core::Vector3 position;
	core::Vector3 rotation;
};

