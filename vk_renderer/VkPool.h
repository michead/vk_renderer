#pragma once

#include <vector>

#include "vulkan\vulkan.h"
#include "Vertex.h"


struct BufferData {
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
};

struct DepthData {
	VkImage image;
	VkImageView imageView;
	VkDeviceMemory imageMemory;
};

struct PipelineData {
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
};


class VkPool {
public:
	VkPool(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device) : 
		instance(instance), physicalDevice(physicalDevice), device(device) 
	{ createInstance(); createDevice(); createSurface(); createDebugCallback(); createSwapchain(); }
	~VkPool() { freeResources(); }

	VkSwapchainKHR getSwapchain() { return swapchain; }
	VkDebugReportCallbackEXT getDebugCallback() { return debugCallback; };
	VkSurfaceKHR getSurface() { return surface; };
	VkPhysicalDevice getPhysicalDevice() { return physicalDevice; };
	VkDevice getDevice() { return device; };
	VkInstance getInstance() { return instance; };

	VkSemaphore createSemaphore();
	VkDescriptorPool createDescriptorPool();
	BufferData createUniformBuffer(size_t bufferSize);
	BufferData createVertexBuffer(std::vector<Vertex> vertices);
	BufferData createIndexBuffer(std::vector<uint32_t> indices);
	DepthData createDepthResources(VkExtent2D extent);
	VkCommandPool createCommandPool(VkSurfaceKHR surface);
	PipelineData createPipeline(
		VkRenderPass renderPass,
		VkDescriptorSetLayout descriptorSetLayout,
		VkExtent2D extent,
		std::vector<char> vs,
		std::vector<char> fs,
		std::vector<char> gs = std::vector<char>());
	VkDescriptorSetLayout createDescriptorSetLayout();
	VkRenderPass createRenderPass(VkRenderPassCreateInfo createInfo);
	VkFramebuffer createFramebuffer(VkFramebufferCreateInfo createInfo);
	VkImageView createSCImageView(VkImage scImage, VkFormat scImageFormat);

	VkSwapchainKHR createSwapchain();
	VkDebugReportCallbackEXT createDebugCallback();
	VkSurfaceKHR createSurface();
	VkDevice createDevice();
	VkInstance createInstance();

private:
	std::vector<VkSemaphore> semaphores;
	std::vector<VkDescriptorPool> descriptorPools;
	std::vector<VkBuffer> buffers;
	std::vector<VkDeviceMemory> deviceMemoryList;
	std::vector<VkBuffer> vertexBuffers;
	std::vector<VkBuffer> indexBuffers;
	std::vector<VkDeviceMemory> vertexDeviceMemoryList;
	std::vector<VkDeviceMemory> indexDeviceMemoryList;
	std::vector<VkImage> depthImages;
	std::vector<VkImageView> depthImageViews;
	std::vector<VkDeviceMemory> depthImageMemoryList;
	std::vector<VkCommandPool> commandPools;
	std::vector<VkPipeline> pipelines;
	std::vector<VkPipelineLayout> pipelineLayouts;
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	std::vector<VkRenderPass> renderPasses;
	std::vector<VkFramebuffer> framebuffers;
	std::vector<VkImageView> scImageViews;

	VkSwapchainKHR swapchain;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkSurfaceKHR surface;
	VkDebugReportCallbackEXT debugCallback;
	VkInstance instance;

	void freeResources();
};