#pragma once

#include <vector>

#include "vulkan\vulkan.h"
#include "Vertex.h"
#include "Config.h"


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
	VkPool(GLFWwindow* window, VkEngineConfig* config)
	{ 
		createInstance(); 
		choosePhysicalDevice();
		createDevice(); 
		createSurface(window); 
		createDebugCallback(); 
		createSwapchain(config->resolution); 
	}

	~VkPool() { freeResources(); }

	VkSwapchainKHR getSwapchain() { return swapchain; }
	VkDebugReportCallbackEXT getDebugCallback() { return debugCallback; };
	VkSurfaceKHR getSurface() { return surface; };
	VkPhysicalDevice getPhysicalDevice() { return physicalDevice; };
	VkDevice getDevice() { return device; };
	VkInstance getInstance() { return instance; };
	VkQueue getGraphicsQueue() { return graphicsQueue; }
	VkQueue getPresentationQueue() { return presentationQueue; }
	std::vector<VkImage>& getSCImages() { return scImages; }
	VkFormat getSCFormat() { return swapchainFormat; }
	VkExtent2D getSCExtent() { return swapchainExtent; }

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

	void createSwapchain(glm::ivec2 resolution);
	void createDebugCallback();
	void createSurface(GLFWwindow* window);
	void choosePhysicalDevice();
	void createDevice();
	void createInstance();

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
	VkDevice device;
	VkSurfaceKHR surface;
	VkDebugReportCallbackEXT debugCallback;
	VkInstance instance;
	VkQueue graphicsQueue;
	VkQueue presentationQueue;

	// These do not need to be collected
	std::vector<VkImage> scImages;
	VkPhysicalDevice physicalDevice;
	VkFormat swapchainFormat;
	VkExtent2D swapchainExtent;

	void freeResources();
};