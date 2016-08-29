#pragma once

#include "Camera.h"

glm::quat Camera::getRotationToTarget(glm::vec3 targetPosition)
{
	glm::vec3 targetDirection = glm::normalize(targetPosition - position);

	return glm::quat(glm::vec3(CAMERA_FORWARD), targetDirection);
}

void Camera::rotateCamera(glm::vec3 rot)
{
	rotation *= glm::quat(rot);

	isDirty = true;
}

void Camera::rotatateCameraAroundTarget(glm::vec3 target, glm::vec3 rot)
{
	isDirty = true;
}

void Camera::panCamera(glm::vec2 pan)
{
	isDirty = true;
}

void Camera::zoomCamera(float zoom)
{
	isDirty = true;
}

void Camera::updateMatrices()
{
	if (!isDirty)
		return;

	viewMatrix = glm::lookAt(position, glm::vec3(CAMERA_FORWARD) * rotation, glm::vec3(CAMERA_UP));
	projMatrix = glm::perspective(fovy, aspectRatio, CAMERA_NEAR, CAMERA_FAR);
	projMatrix[1][1] *= -1;
}