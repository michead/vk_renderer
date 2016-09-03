#pragma once

#include "VkApp.h"
#include "VkObjWrapper.h"
#include "GeomStructs.h"


class SceneElem {
	friend class Scene;

public:
	SceneElem() { }
	~SceneElem() { }

	std::string getName() const { return name; }
	VK_WRAP(VkBuffer)& getVertexBuffer() { return vertexBuffer; }
	VK_WRAP(VkBuffer)& getIndexBuffer() { return indexBuffer; }
	Mesh& getMesh() { return mesh; }
	Material& getMaterial() { return material; }

private:
	std::string name;
	Mesh mesh;
	Material material;

	VK_WRAP(VkBuffer) vertexBuffer { VkApp::getDevice(), vkDestroyBuffer };
	VK_WRAP(VkDeviceMemory) vertexBufferMemory { VkApp::getDevice(), vkFreeMemory };
	VK_WRAP(VkBuffer) indexBuffer{ VkApp::getDevice(), vkDestroyBuffer };
	VK_WRAP(VkDeviceMemory) indexBufferMemory{ VkApp::getDevice(), vkFreeMemory };

	virtual void initVertexBuffer();
	virtual void initIndexBuffer();
};