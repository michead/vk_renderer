#pragma once

#include "VkEngine.h"
#include "Material.h"


class Mesh {
	friend class VkEngine;
	friend struct Scene;
	friend class GeometryPass;

public:
	std::string getName() const { return name; }
	VkBuffer getVertexBuffer() { return vertexBuffer; }
	VkBuffer getIndexBuffer() { return indexBuffer; }
	Material* getMaterial() const { return material; }

	virtual void initBuffers() { initVertexBuffer(); initIndexBuffer(); }

private:
	std::string name;
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	Material* material;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	virtual void initVertexBuffer();
	virtual void initIndexBuffer();
};