#include "SceneElem.h"

void SceneElem::initVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(mesh.vertices[0]) * mesh.vertices.size();

	VkObjWrapper<VkBuffer> stagingBuffer { VkApp::getDevice(), vkDestroyBuffer };
	VkObjWrapper<VkDeviceMemory> stagingBufferMemory { VkApp::getDevice(), vkFreeMemory };
	createBuffer(
		VkApp::getPhysicalDevice(), 
		VkApp::getDevice(), 
		bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
		stagingBuffer, 
		stagingBufferMemory);

	void* data;
	vkMapMemory(VkApp::getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, mesh.vertices.data(), (size_t) bufferSize);
	vkUnmapMemory(VkApp::getDevice(), stagingBufferMemory);

	createBuffer(
		VkApp::getPhysicalDevice(), 
		VkApp::getDevice(), 
		bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
		vertexBuffer, 
		vertexBufferMemory);

	copyBuffer(VkApp::getDevice(), VkApp::getCommandPool(), VkApp::getGraphicsQueue(), stagingBuffer, vertexBuffer, bufferSize);
}

void SceneElem::initIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(mesh.indices[0]) * mesh.indices.size();

	VkObjWrapper<VkBuffer> stagingBuffer{ VkApp::getDevice(), vkDestroyBuffer };
	VkObjWrapper<VkDeviceMemory> stagingBufferMemory { VkApp::getDevice(), vkFreeMemory };
	createBuffer(
		VkApp::getPhysicalDevice(), 
		VkApp::getDevice(), 
		bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
		stagingBuffer, 
		stagingBufferMemory);

	void* data;
	vkMapMemory(VkApp::getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, mesh.indices.data(), (size_t) bufferSize);
	vkUnmapMemory(VkApp::getDevice(), stagingBufferMemory);

	createBuffer(
		VkApp::getPhysicalDevice(), 
		VkApp::getDevice(), 
		bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
		indexBuffer, 
		indexBufferMemory);

	copyBuffer(
		VkApp::getDevice(), 
		VkApp::getCommandPool(), 
		VkApp::getGraphicsQueue(), 
		stagingBuffer, 
		indexBuffer, 
		bufferSize);
}