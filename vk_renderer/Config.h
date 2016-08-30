#pragma once

#include "glm\glm.hpp"

#define RENDER_TARGET_RESOLUTION "RENDER_TARGET_RESOLUTION"

struct VkAppConfig {
	glm::vec2 resolution;

	static glm::vec2 parseResolution(std::string resolution)
	{
		int sepPos = resolution.find('x');
		
		int width = std::atoi(resolution.substr(0, sepPos).c_str());
		int height = std::atoi(resolution.substr(sepPos).c_str());

		return glm::vec2(width, height);
	}
};