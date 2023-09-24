#pragma once

#include "Camera.h"

class BasicCamera : public Camera
{
public:
	BasicCamera();
	BasicCamera(BasicCamera&&) = default;
	virtual ~BasicCamera() {};

	virtual void reset() override;
	virtual void setup() override;

	virtual void rotateStart() override;
	virtual void rotate(Vector2 angle_offset) override;
	virtual void rotateEnd() override;

	virtual void moveStart() override;
	virtual void move(Vector3 position_offset) override;
	virtual void moveEnd() override;

	virtual void zoom(float zoom) override;

	void rotateX(float angle);
	void rotateY(float angle);
	void rotateZ(float angle);

	void moveForward(float distance);
	void moveUpward(float distance);
	void strafe(float distance);

protected:
	Vector3 viewDirection;
	Vector3 rightVector;
	Vector3 upVector;
	Vector3 position;
	Vector3 rotation;
};

