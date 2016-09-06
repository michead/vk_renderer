#pragma once

#include "VkEngine.h"

#define SHADER_MAIN "main"


struct Texture;


struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};



class RenderPass {
public:
	RenderPass(std::string vsPath, std::string gsPath, std::string fsPath) : vsPath(vsPath), gsPath(gsPath), fsPath(fsPath) { }
	~RenderPass() { }

	virtual void init();
	virtual VkResult run();
	virtual void updateData();

protected:
	std::string vsPath;
	std::string gsPath;
	std::string fsPath;

	std::vector<VkFramebuffer> swapchainFramebuffers;
	
	VkWrap<VkImage> depthImage { VkEngine::getInstance().getDevice(), vkDestroyImage };
	VkWrap<VkImageView> depthImageView { VkEngine::getInstance().getDevice(), vkDestroyImageView };
	VkWrap<VkDeviceMemory> depthImageMemory { VkEngine::getInstance().getDevice(), vkFreeMemory };

	VkRenderPass renderPass;
	VkWrap<VkPipelineLayout> pipelineLayout { VkEngine::getInstance().getDevice(), vkDestroyPipelineLayout };
	VkPipeline graphicsPipeline;

	std::vector<VkCommandBuffer> commandBuffers;

	VkWrap<VkBuffer> uniformStagingBuffer { VkEngine::getInstance().getDevice(), vkDestroyBuffer };
	VkWrap<VkDeviceMemory> uniformStagingBufferMemory { VkEngine::getInstance().getDevice(), vkFreeMemory };
	VkWrap<VkBuffer> uniformBuffer { VkEngine::getInstance().getDevice() , vkDestroyBuffer};
	VkWrap<VkDeviceMemory> uniformBufferMemory { VkEngine::getInstance().getDevice(), vkFreeMemory };

	std::vector<VkDescriptorSetLayout> layouts;
	VkDescriptorSet descriptorSet;

	static void getDescriptorSetLayouts();

	virtual void initUniformBuffer();
	virtual void initDescriptorSet();
	virtual void initCommandBuffers();

	virtual void initAttachments();
	virtual void initFramebuffers();
	virtual void initDepthResources();
	virtual void initGraphicsPipeline();
};