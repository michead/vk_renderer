#pragma once

#include "Vertex.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm\glm.hpp"


class Quad {
public:
	Quad() { initVertexBuffer(); initIndexBuffer(); }
	~Quad() { }

	static const std::vector<glm::vec3> positions;
	static const std::vector<uint32_t> indices;
	static const std::vector<glm::vec2> uvs;
	static const std::vector<Vertex> vertices;

	VkBuffer getVertexBuffer() { return vertexBuffer; }
	VkBuffer getIndexBuffer() { return indexBuffer; }

private:
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	void initVertexBuffer();
	void initIndexBuffer();
};