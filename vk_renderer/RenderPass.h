#pragma once

#include "VkEngine.h"


struct Texture;


struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};



class RenderPass {
public:
	RenderPass(std::string vsPath, std::string fsPath) : vsPath(vsPath), fsPath(fsPath) { }
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
	
	VkWrap<VkImage> depthImage { VkEngine::getEngine().getDevice(), vkDestroyImage };
	VkWrap<VkImageView> depthImageView { VkEngine::getEngine().getDevice(), vkDestroyImageView };
	VkWrap<VkDeviceMemory> depthImageMemory { VkEngine::getEngine().getDevice(), vkFreeMemory };

	VkRenderPass renderPass;
	VkWrap<VkPipelineLayout> pipelineLayout { VkEngine::getEngine().getDevice(), vkDestroyPipelineLayout };
	VkPipeline graphicsPipeline;

	std::vector<VkCommandBuffer> commandBuffers;

	VkWrap<VkBuffer> uniformStagingBuffer { VkEngine::getEngine().getDevice(), vkDestroyBuffer };
	VkWrap<VkDeviceMemory> uniformStagingBufferMemory { VkEngine::getEngine().getDevice(), vkFreeMemory };
	VkWrap<VkBuffer> uniformBuffer { VkEngine::getEngine().getDevice() , vkDestroyBuffer};
	VkWrap<VkDeviceMemory> uniformBufferMemory { VkEngine::getEngine().getDevice(), vkFreeMemory };

	VkDescriptorSetLayout layout;
	VkDescriptorSet descriptorSet;

	virtual void initUniformBuffer();
	virtual void initDescriptorSet();
	virtual void initDescriptorSetLayout();
	virtual void initCommandBuffers();

	virtual void initAttachments();
	virtual void initFramebuffers();
	virtual void initTextures();
	virtual void initDepthResources();
	virtual void initGraphicsPipeline();
};