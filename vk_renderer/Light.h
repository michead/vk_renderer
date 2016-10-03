#pragma once

#include "Camera.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm\glm.hpp>


enum LightType {
	POINT,
	NUM_LIGHT_TYPES
};

struct Light {
	LightType type = POINT;
	glm::vec3 position;
	glm::vec3 intensity;

	glm::mat4 getViewMatrix(glm::vec3 target) const { return glm::lookAt(position, target, CAMERA_UP); }
};