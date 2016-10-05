#pragma once

#include "Camera.h"

#include "MathUtils.h"


void Camera::rotate(glm::vec2 rot)
{
	float phi = atan2(frame.zAxis.z, frame.zAxis.x) + rot.x;
	float theta = CLAMP(acos(frame.zAxis.y) + rot.y, MIN_THETA, glm::pi<float>() - MIN_THETA);
	
	glm::vec3 newZ = glm::vec3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
	glm::vec3 newTarget = frame.origin + newZ * focus;
	
	frame = Frame::lookAtFrame(frame.origin, newTarget, up);
	target = newTarget;
}

void Camera::rotateAroundTarget(glm::vec2 rot)
{
	auto phi = atan2(frame.zAxis.z, frame.zAxis.x) + rot.x;
	auto theta = CLAMP(acos(frame.zAxis.y) - rot.y, MIN_THETA, glm::pi<float>() - MIN_THETA);
	auto newZ = glm::vec3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
	auto newCenter = frame.origin - frame.zAxis * focus;
	auto newO = newCenter + newZ * focus;
	frame = Frame::lookAtFrame(newO, newCenter, up);
	focus = glm::distance(newO, newCenter);
}

void Camera::pan(glm::vec2 pan)
{
	frame.origin += frame.xAxis * pan.x + frame.yAxis * pan.y;
}

void Camera::zoom(float zoom)
{
	glm::vec3 c = frame.origin - frame.zAxis * focus;
	focus = MAX(focus + zoom, MIN_FOCUS);
	frame.origin = c + frame.zAxis * focus;
}

void Camera::initMatrices()
{
	Camera::lookAtCamera(this, frame.origin, target, up);

	viewMatrix = glm::lookAt(frame.origin, target, up);
	projMatrix = glm::perspective(fovy, aspectRatio, CAMERA_NEAR, CAMERA_FAR);
	projMatrix[1][1] *= -1;
}

void Camera::updateViewMatrix()
{
	viewMatrix = glm::lookAt(frame.origin, target, up);
}