#pragma once

#include "VkEngine.h"
#include "GeomStructs.h"


class SceneElem {
	friend struct Scene;

public:
	SceneElem() { }
	~SceneElem() { cleanup(); }

	std::string getName() const { return name; }
	VkBuffer& getVertexBuffer() { return vertexBuffer; }
	VkBuffer& getIndexBuffer() { return indexBuffer; }
	Mesh& getMesh() { return mesh; }
	Material& getMaterial() { return material; }

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

	void cleanup();
};