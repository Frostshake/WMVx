#pragma once

#include "Camera.h"
#include "core/utility/Matrix.h"
#include <glm/glm.hpp>

class ArcBallCamera : public Camera
{
public:
	ArcBallCamera();
	ArcBallCamera(ArcBallCamera&&) = default;
	virtual ~ArcBallCamera() {};

	static constexpr const char* identifier = "arcball";

	virtual void reset() override;
	virtual void setup() override;

	virtual void leftMouseStart() override;
	virtual void leftMouse(float change_x, float change_y, float factor) override;
	virtual void leftMouseEnd() override;

	virtual void rightMouseStart() override;
	virtual void rightMouse(float change_x, float change_y, float factor) override;
	virtual void rightMouseEnd() override;

	virtual void scroll(float change, float factor) override;

protected:

	glm::mat4x4 m_viewMatrix;
	glm::vec3 m_upVector;

	glm::vec3 m_LookAt;
	glm::vec3 m_eye;


};

