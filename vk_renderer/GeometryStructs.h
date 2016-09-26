#pragma once

#include <glm\glm.hpp>

#include "Texture.h"
#include "Vertex.h"


enum LightType {
	POINT,
	NUM_LIGHT_TYPES
};

struct Light {
	LightType type = POINT;
	glm::vec3 pos;
	glm::vec3 intensity;
};

struct Material {
	std::string name = "";

	glm::vec3  ke = glm::vec3();
	glm::vec3  kd = glm::vec3();
	glm::vec3  ks = glm::vec3();
	glm::vec3  kr = glm::vec3();
	float      rs = 0.15f;

	Texture*    keTxt = nullptr;
	Texture*    kdTxt = nullptr;
	Texture*    ksTxt = nullptr;
	Texture*    rsTxt = nullptr;
	Texture*    krTxt = nullptr;
	Texture*    normTxt = nullptr;

	float       opacity = 1;
	float		translucency = 0;
	float		subsurfWidth = 0;
};

struct Mesh {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};