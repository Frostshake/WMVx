#include "stdafx.h"
#include "ArcBallCamera.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/quaternion.hpp>
#include "core/utility/Logger.h"

using namespace core;

ArcBallCamera::ArcBallCamera()
{
	reset();
}

void ArcBallCamera::reset()
{
	m_LookAt = glm::vec3{ 0.f, 0.f, 0.f };
	m_upVector = glm::vec3{ 0.f, 1.f, 0.f };
	m_eye = glm::vec3{ 0.0f, 0.0f, 1.0f };
}

void ArcBallCamera::setup()
{
	glLoadIdentity();

	m_viewMatrix = glm::lookAt(m_eye, m_LookAt, m_upVector);
	gluLookAt(m_eye.x, m_eye.y, m_eye.z, m_LookAt.x, m_LookAt.y, m_LookAt.z, m_upVector.x, m_upVector.y, m_upVector.z);

}

void ArcBallCamera::key(float change_x, float change_y, bool alternative, float factor)
{
	const float scale = 4.f;
	if (alternative) {
		leftMouse(change_x * scale, change_y * scale, factor);
	}
	else {
		rightMouse(change_x * scale, change_y * scale, factor);
	}
}

void ArcBallCamera::leftMouseStart()
{
}

void ArcBallCamera::leftMouse(float change_x, float change_y, float factor)
{
	glm::vec4 eye(m_eye.x, m_eye.y, m_eye.z, 1);
	glm::vec4 center(m_LookAt.x, m_LookAt.y, m_LookAt.z, 1);

	glm::mat4x4 rotationMatrixX(1.0f);
	rotationMatrixX = glm::rotate(rotationMatrixX, (change_x * factor) / 100.f, m_upVector);
	center = (rotationMatrixX * (center - eye)) + eye;

	glm::mat4x4 rotationMatrixY(1.0f);
	rotationMatrixY = glm::rotate(rotationMatrixY, (change_y * factor) / 100.f, (glm::vec3)glm::transpose(m_viewMatrix)[0]);
	center = (rotationMatrixY * (center - eye)) + eye;

	m_LookAt.x = center.x;
	m_LookAt.y = center.y;
	m_LookAt.z = center.z;
}

void ArcBallCamera::leftMouseEnd()
{
}

void ArcBallCamera::rightMouseStart()
{
}

void ArcBallCamera::rightMouse(float change_x, float change_y, float factor)
{
	glm::vec4 eye(m_eye.x, m_eye.y, m_eye.z, 1);
	glm::vec4 center(m_LookAt.x, m_LookAt.y, m_LookAt.z, 1);

	glm::mat4x4 rotationMatrixX(1.0f);
	rotationMatrixX = glm::rotate(rotationMatrixX, change_x * factor / 100.f, m_upVector);
	eye = (rotationMatrixX * (eye - center)) + center;

	glm::mat4x4 rotationMatrixY(1.0f);
	rotationMatrixY = glm::rotate(rotationMatrixY, change_y * factor / 100.f, (glm::vec3)glm::transpose(m_viewMatrix)[0]);
	eye = (rotationMatrixY * (eye - center)) + center;

	m_eye.x = eye.x;
	m_eye.y = eye.y;
	m_eye.z = eye.z;
}

void ArcBallCamera::rightMouseEnd()
{
}

void ArcBallCamera::scroll(float scroll, float factor)
{
	constexpr float min_zoom = 0.5f;
	constexpr float max_zoom = 100.0f;

	const float len = glm::length(m_eye - m_LookAt);
	float new_len = len + ((scroll * factor) / 2);
	new_len = std::clamp(new_len, min_zoom, max_zoom);
	m_eye = glm::lerp(m_LookAt, m_eye, new_len / len);	
}


