#pragma once

#include "VkEngine.h"
#include "Material.h"
#include "MathUtils.h"


struct MeshUniformBufferObject {
	glm::mat4 model;
};

class Mesh {
	friend class VkEngine;
	friend struct Scene;
	friend class ShadowPass;
	friend class GeometryPass;

public:
	std::string getName() const { return name; }
	VkBuffer getVertexBuffer() const { return vertexBuffer; }
	VkBuffer getIndexBuffer() const { return indexBuffer; }
	Material* getMaterial() const { return material; }
	glm::mat4 getModelMatrix() const { return frame.toMatrix(); }

	void initBuffers() { initVertexBuffer(); initIndexBuffer(); }

private:
	std::string name;
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	Material* material;
	Frame frame = { IDENTITY_FRAME };

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	void initVertexBuffer();
	void initIndexBuffer();
};