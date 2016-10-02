#pragma once

#include "Camera.h"


void Camera::rotate(glm::vec2 rot)
{
	float phi = atan2(frame.zAxis.z, frame.zAxis.x) + rot.x;
	float theta = CLAMP(acos(frame.zAxis.y) + rot.y, MIN_THETA, glm::pi<float>() - MIN_THETA);
	
	float dist = getFocus();

	glm::vec3 newZ = glm::vec3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
	glm::vec3 newTarget = frame.origin + newZ * dist;
	
	frame = Frame::lookAtFrame(frame.origin, newTarget, CAMERA_UP);
	target = newTarget;
}

void Camera::rotateAroundTarget(glm::vec2 rot)
{
	float phi = atan2(frame.zAxis.z, frame.zAxis.x) + rot.x;
	float theta = CLAMP(acos(frame.zAxis.y) + rot.y, MIN_THETA, glm::pi<float>() - MIN_THETA);

	glm::vec3 newZ = glm::vec3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
	glm::vec3 newOrigin = target - newZ * getFocus();

	frame = Frame::lookAtFrame(newOrigin, target, CAMERA_UP);
}

void Camera::pan(glm::vec2 pan)
{
	frame.origin += frame.xAxis * pan.x + frame.yAxis * pan.y;
}

void Camera::zoom(float zoom)
{
	frame.origin += frame.zAxis * zoom * CAMERA_DOLLY_SCALE;
}

void Camera::updateMatrices()
{
	target = frame.origin + getFocus() * frame.zAxis;

	viewMatrix = glm::lookAt(frame.origin, target, CAMERA_UP);
	projMatrix = glm::perspective(fovy, aspectRatio, CAMERA_NEAR, CAMERA_FAR);
	projMatrix[1][1] *= -1;
}