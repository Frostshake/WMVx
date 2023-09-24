#include "stdafx.h"
#include "ArcBallCamera.h"

ArcBallCamera::ArcBallCamera()
{
	reset();
}

void ArcBallCamera::reset()
{
	m_distance = 5.;
	m_lookAt.reset();
	m_modelCenter.reset();
	m_eye = Vector3(0.0f, 0.0f, 1.0f);
}

void ArcBallCamera::setup()
{
	glLoadIdentity();


	gluLookAt(m_eye.x, m_eye.y, m_eye.z, m_modelCenter.x, m_modelCenter.y, m_modelCenter.z, 0, 1, 0);

}

void ArcBallCamera::rotateStart()
{
}

void ArcBallCamera::rotate(Vector2 angle_offset)
{
	m_modelCenter.x += 0 - (angle_offset.x / 5);
	m_modelCenter.y += (angle_offset.y / 5);
}

void ArcBallCamera::rotateEnd()
{
}

void ArcBallCamera::moveStart()
{
}

void ArcBallCamera::move(Vector3 position_offset)
{
	m_eye.x += position_offset.x;
	m_eye.y += position_offset.z;

	//TODO handle moving camera as sphere
}

void ArcBallCamera::moveEnd()
{
}

void ArcBallCamera::zoom(float zoom)
{
	constexpr float min_zoom = 1;
	constexpr float max_zoom = 70;

	m_distance += ((m_distance - min_zoom) / 5.0 * zoom);

	m_distance = std::clamp(m_distance, min_zoom, max_zoom);

	//TODO handle zoom	
}


