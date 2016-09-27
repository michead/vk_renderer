#pragma once

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
};