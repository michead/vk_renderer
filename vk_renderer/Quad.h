#pragma once

#include "Vertex.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm\glm.hpp"


class Quad {
public:
	Quad() { initVertexBuffer(); initIndexBuffer(); }
	~Quad() { }

	static std::vector<Vertex> vertices;
	static std::vector<uint32_t> indices;

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