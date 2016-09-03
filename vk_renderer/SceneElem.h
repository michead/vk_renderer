#pragma once

#include "VkEngine.h"
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

	VK_WRAP(VkBuffer) vertexBuffer { VkEngine::getInstance()->getDevice(), vkDestroyBuffer };
	VK_WRAP(VkDeviceMemory) vertexBufferMemory { VkEngine::getInstance()->getDevice(), vkFreeMemory };
	VK_WRAP(VkBuffer) indexBuffer { VkEngine::getInstance()->getDevice(), vkDestroyBuffer };
	VK_WRAP(VkDeviceMemory) indexBufferMemory { VkEngine::getInstance()->getDevice(), vkFreeMemory };

	virtual void initVertexBuffer();
	virtual void initIndexBuffer();
};