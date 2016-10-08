#pragma once

#include <vector>

#include "vulkan\vulkan.h"
#include "Vertex.h"
#include "Config.h"
#include "GBuffer.h"

#define MAX_DESCRIPTOR_SETS			32
#define POOL_UNIFORM_BUFFER_SIZE	40
#define POOL_COMBINED_SAMPLER_SIZE	40

struct BufferData {
	VkBuffer buffer;
	VkDeviceMemory bufferMemory;
};

struct ImageData {
	VkImage image;
	VkImageView imageView;
	VkDeviceMemory imageMemory;
	VkSampler sampler;
};

struct PipelineData {
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
};


class VkPool {
public:
	VkPool(GLFWwindow* window, Config* config)
	{ 
		createInstance(); 
		createDebugCallback();
		createSurface(window);
		setPhysicalDevice();
		createDevice();  
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
	std::vector<VkImage>& getSwapchainImages() { return swapchainImages; }
	VkFormat getSwapchainFormat() { return swapchainFormat; }
	VkExtent2D getSwapchainExtent() { return swapchainExtent; }

	VkSemaphore createSemaphore();
	VkDescriptorPool createDescriptorPool(
		uint32_t bufferDescriptorCount,
		uint32_t imageSamplerDescriptorCount,
		uint32_t maxSets = MAX_DESCRIPTOR_SETS);
	std::vector<BufferData> createUniformBuffer(VkDeviceSize bufferSize, bool createStaging);
	BufferData createVertexBuffer(std::vector<Vertex> vertices);
	BufferData createIndexBuffer(std::vector<uint32_t> indices);
	ImageData createDepthResources();
	VkCommandPool createCommandPool();
	PipelineData createPipeline(
		VkRenderPass renderPass,
		VkDescriptorSetLayout descriptorSetLayout,
		VkExtent2D extent,
		std::vector<char> vs,
		std::vector<char> fs,
		std::vector<char> gs = std::vector<char>(),
		uint16_t numColorAttachments = GBufferAttachmentType::NUM_TYPES - 1);
	VkDescriptorSetLayout createDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding>& bindings);
	VkRenderPass createRenderPass(VkRenderPassCreateInfo createInfo);
	VkFramebuffer createFramebuffer(VkFramebufferCreateInfo createInfo);
	VkImageView createSwapchainImageView(VkImage swapchainImage);
	ImageData createTextureResources(void* pixels, unsigned int texWidth, unsigned int texHeight, bool highPrec = false);
	GBufferAttachment createGBufferAttachment(GBufferAttachmentType type, bool toBeSampled = true);
	VkFence createFence();

	void createSwapchain(glm::ivec2 resolution);
	void createDebugCallback();
	void createSurface(GLFWwindow* window);
	void setPhysicalDevice();
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
	std::vector<VkSampler> depthSamplers;
	std::vector<VkImage> depthImages;
	std::vector<VkImageView> depthImageViews;
	std::vector<VkDeviceMemory> depthImageMemoryList;
	std::vector<VkCommandPool> commandPools;
	std::vector<VkPipeline> pipelines;
	std::vector<VkPipelineLayout> pipelineLayouts;
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	std::vector<VkRenderPass> renderPasses;
	std::vector<VkFramebuffer> framebuffers;
	std::vector<VkImageView> swapchainImageViews;
	std::vector<VkSampler> textureSamplers;
	std::vector<VkImage> textureImages;
	std::vector<VkImageView> textureImageViews;
	std::vector<VkDeviceMemory> textureImageMemoryList;
	std::vector<VkImage> offscreenImages;
	std::vector<VkImageView> offscreenImageViews;
	std::vector<VkDeviceMemory> offscreenImageMemoryList;
	std::vector<VkSampler> offscreenImageSamplers;
	std::vector<VkShaderModule> shaderModules;
	std::vector<VkFence> fences;

	VkSwapchainKHR swapchain;
	VkDevice device;
	VkSurfaceKHR surface;
	VkDebugReportCallbackEXT debugCallback;
	VkInstance instance;

	// These do not need to be collected
	std::vector<VkImage> swapchainImages;
	VkPhysicalDevice physicalDevice;
	VkFormat swapchainFormat;
	VkExtent2D swapchainExtent;
	VkQueue graphicsQueue;
	VkQueue presentationQueue;

	void freeResources();
};