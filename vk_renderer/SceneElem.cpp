#include "SceneElem.h"

#include "VkUtils.h"


void SceneElem::initVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(mesh.vertices[0]) * mesh.vertices.size();

	VkWrap<VkBuffer> stagingBuffer { VkEngine::getEngine().getDevice(), vkDestroyBuffer };
	VkWrap<VkDeviceMemory> stagingBufferMemory { VkEngine::getEngine().getDevice(), vkFreeMemory };
	
	createBuffer(
		VkEngine::getEngine().getPhysicalDevice(), 
		VkEngine::getEngine().getDevice(), 
		bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
		stagingBuffer.get(), 
		stagingBufferMemory.get());

	void* data;
	VK_CHECK(vkMapMemory(VkEngine::getEngine().getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data));
	memcpy(data, mesh.vertices.data(), (size_t) bufferSize);
	vkUnmapMemory(VkEngine::getEngine().getDevice(), stagingBufferMemory);

	createBuffer(
		VkEngine::getEngine().getPhysicalDevice(), 
		VkEngine::getEngine().getDevice(), 
		bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
		vertexBuffer, 
		vertexBufferMemory);

	copyBuffer(
		VkEngine::getEngine().getDevice(), 
		VkEngine::getEngine().getCommandPool(), 
		VkEngine::getEngine().getGraphicsQueue(), 
		stagingBuffer, 
		vertexBuffer, 
		bufferSize);
}

void SceneElem::initIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(mesh.indices[0]) * mesh.indices.size();

	VkWrap<VkBuffer> stagingBuffer { VkEngine::getEngine().getDevice(), vkDestroyBuffer };
	VkWrap<VkDeviceMemory> stagingBufferMemory { VkEngine::getEngine().getDevice(), vkFreeMemory };
	createBuffer(
		VkEngine::getEngine().getPhysicalDevice(), 
		VkEngine::getEngine().getDevice(), 
		bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
		stagingBuffer.get(), 
		stagingBufferMemory.get());

	void* data;
	VK_CHECK(vkMapMemory(VkEngine::getEngine().getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data));
	memcpy(data, mesh.indices.data(), (size_t) bufferSize);
	vkUnmapMemory(VkEngine::getEngine().getDevice(), stagingBufferMemory);

	createBuffer(
		VkEngine::getEngine().getPhysicalDevice(), 
		VkEngine::getEngine().getDevice(), 
		bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
		indexBuffer, 
		indexBufferMemory);

	copyBuffer(
		VkEngine::getEngine().getDevice(), 
		VkEngine::getEngine().getCommandPool(), 
		VkEngine::getEngine().getGraphicsQueue(), 
		stagingBuffer, 
		indexBuffer, 
		bufferSize);
}

void SceneElem::deleteBuffers()
{
	vkFreeMemory(VkEngine::getEngine().getDevice(), indexBufferMemory, nullptr);
	vkDestroyBuffer(VkEngine::getEngine().getDevice(), indexBuffer, nullptr);
	vkFreeMemory(VkEngine::getEngine().getDevice(), vertexBufferMemory, nullptr);
	vkDestroyBuffer(VkEngine::getEngine().getDevice(), vertexBuffer, nullptr);
}