#pragma once

#include "Camera.h"
#include "core/utility/Matrix.h"

class ArcBallCamera : public Camera
{
public:
	ArcBallCamera();
	ArcBallCamera(ArcBallCamera&&) = default;
	virtual ~ArcBallCamera() {};

	virtual void reset() override;
	virtual void setup() override;

	virtual void rotateStart() override;
	virtual void rotate(core::Vector2 angle_offset) override;
	virtual void rotateEnd() override;

	virtual void moveStart() override;
	virtual void move(core::Vector3 position_offset) override;
	virtual void moveEnd() override;

	virtual void zoom(float zoom) override;

protected:

	core::Vector3 m_lookAt;
	core::Vector3 m_modelCenter;
	core::Vector3 m_eye;

	float m_distance;


};

