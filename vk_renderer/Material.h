#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm\glm.hpp>

#include "Texture.h"
#include "Vertex.h"


struct Material {
	int			id;

	glm::vec3	ke = glm::vec3();
	glm::vec3	kd = glm::vec3();
	glm::vec3	ks = glm::vec3();
	glm::vec3	kr = glm::vec3();
	
	float		rs = 0.15f;
	float		ns = 1;

	Texture*    keMap = nullptr;
	Texture*    kdMap = nullptr;
	Texture*    ksMap = nullptr;
	Texture*    rsMap = nullptr;
	Texture*    krMap = nullptr;
	Texture*    normalMap = nullptr;

	float       opacity = 1;
	float		translucency = 0;
	float		subsurfWidth = 0;
};