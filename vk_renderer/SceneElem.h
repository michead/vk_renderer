#pragma once

#include "VkEngine.h"
#include "GeometryStructs.h"


class SceneElem {
	friend class VkEngine;
	friend struct Scene;

public:
	SceneElem() { }
	~SceneElem() { }

	std::string getName() const { return name; }
	VkBuffer getVertexBuffer() { return vertexBuffer; }
	VkBuffer getIndexBuffer() { return indexBuffer; }
	Mesh& getMesh() { return mesh; }
	Material& getMaterial() { return material; }

	virtual void initBuffers() { initVertexBuffer(); initIndexBuffer(); }

private:
	std::string name;
	Mesh mesh;
	Material material;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	virtual void initVertexBuffer();
	virtual void initIndexBuffer();
};