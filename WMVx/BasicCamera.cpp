#include "stdafx.h"
#include "BasicCamera.h"
#include "OpenGL.h"

BasicCamera::BasicCamera() :Camera()
{
	reset();
}

void BasicCamera::reset()
{
	//Init with standard OGL values:
	position = Vector3(0.0f, 0.0f, 1.0f);
	viewDirection = Vector3(0.0f, 0.0f, -1.0f);
	rightVector = Vector3(1.0f, 0.0f, 0.0f);
	upVector = Vector3(0.0f, 1.0f, 0.0f);

	rotation = Vector3(0, 0, 0);
}

void BasicCamera::setup()
{
	Vector3 viewPoint = position + viewDirection;

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix

	gluLookAt(position.x, position.y, position.z,			// Specifies the position of the eye point.
		viewPoint.x, viewPoint.y, viewPoint.z,				// Specifies the position of the reference point.
		upVector.x, upVector.y, upVector.z);		// Specifies the direction of the up vector.


	//TODO should matrix mode be reset?
}

void BasicCamera::rotateStart()
{
}

void BasicCamera::rotate(Vector2 angle_offset)
{
	rotateX(angle_offset.y);
	rotateY(angle_offset.x);
}

void BasicCamera::rotateEnd()
{
}

void BasicCamera::moveStart()
{
}

void BasicCamera::move(Vector3 position_offset)
{
	//moveForward(position_offset.y);
	strafe(position_offset.z);
	moveUpward(position_offset.x);
}

void BasicCamera::moveEnd()
{
}

void BasicCamera::zoom(float zoom)
{
	moveForward(zoom);
}


void BasicCamera::rotateX(float angle)
{
	rotation.x += angle;

	//Rotate viewdir around the right vector:
	viewDirection = ((viewDirection * cosf(angle * PIOVER180f)) + (upVector * sinf(angle * PIOVER180f))).normalize();

	//now compute the new UpVector (by cross product)
	//upVector = (viewDirection % rightVector) * -1;
}

void BasicCamera::rotateY(float angle)
{
	rotation.y += angle;

	//Rotate viewdir around the up vector:
	viewDirection = ((viewDirection * cosf(angle * PIOVER180f)) - (rightVector * sinf(angle * PIOVER180f))).normalize();

	//now compute the new RightVector (by cross product)
	rightVector = (viewDirection % upVector);
}

void BasicCamera::rotateZ(float angle)
{
	rotation.z += angle;

	//Rotate viewdir around the right vector:
	rightVector = (rightVector * cosf(angle * PIOVER180f) + upVector * sinf(angle * PIOVER180f)).normalize();

	//now compute the new UpVector (by cross product)
	//upVector = (viewDirection % rightVector) * -1;
}

void BasicCamera::moveForward(float distance)
{
	position += (viewDirection * -distance);
}

void BasicCamera::moveUpward(float distance)
{
	position += (rightVector * distance);
}

void BasicCamera::strafe(float distance)
{
	position += (upVector * distance);
}
