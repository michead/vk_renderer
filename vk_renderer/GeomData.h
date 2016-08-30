#pragma once

#include "vulkan\vulkan.h"
#include "glm\glm.hpp"
#include "glm\gtx\hash.hpp"
#include "Camera.h"
#include <array>
#include <vector>

struct Vertex {
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 texCoord;

	bool operator==(const Vertex& other) const
	{
		return position == other.position && color == other.color && texCoord == other.texCoord;
	}

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.position) ^
					(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
					(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

enum LightType {
	POINT
};

struct Light {
	LightType type = POINT;
	glm::vec3 pos;
	glm::vec3 color = glm::vec3(1);
};

struct Texture {

};

struct Material {
	std::string id = "";

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
	Material material;
};

struct Scene {
	std::vector<Mesh> meshes;
	std::vector<Light> lights;
	Camera camera;
};

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};