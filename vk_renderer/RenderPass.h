#pragma once

#include "VkEngine.h"
#include "VkObjWrapper.h"
#include "Common.h"
#include "Texture.h"

#define SHADER_MAIN "main"


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

	VkEngine engine;

	VK_VEC_WRAP(VkFramebuffer) swapchainFramebuffers;
	
	VK_WRAP(VkImage) depthImage { VkEngine::getInstance()->getDevice(), vkDestroyImage };
	VK_WRAP(VkImageView) depthImageView { VkEngine::getInstance()->getDevice(), vkDestroyImageView };
	VK_WRAP(VkDeviceMemory) depthImageMemory { VkEngine::getInstance()->getDevice(), vkFreeMemory };

	VK_WRAP(VkRenderPass) renderPass { VkEngine::getInstance()->getDevice(), vkDestroyRenderPass };
	VK_WRAP(VkPipelineLayout) pipelineLayout { VkEngine::getInstance()->getDevice(), vkDestroyPipelineLayout };
	VK_WRAP(VkPipeline) graphicsPipeline { VkEngine::getInstance()->getDevice(), vkDestroyPipeline };

	std::vector<VkCommandBuffer> commandBuffers;

	VK_WRAP(VkBuffer) uniformStagingBuffer { VkEngine::getInstance()->getDevice(), vkDestroyBuffer };
	VK_WRAP(VkDeviceMemory) uniformStagingBufferMemory { VkEngine::getInstance()->getDevice(), vkFreeMemory };
	VK_WRAP(VkBuffer) uniformBuffer { VkEngine::getInstance()->getDevice(), vkDestroyBuffer };
	VK_WRAP(VkDeviceMemory) uniformBufferMemory { VkEngine::getInstance()->getDevice(), vkFreeMemory };

	std::vector<VkDescriptorSetLayout> layouts;
	VkDescriptorSet descriptorSet;

	static std::vector<VkDescriptorSetLayout>& getDescriptorSetLayouts(std::vector<Texture*> textures);

	virtual void initGraphicsPipeline();

	virtual void createUniformBuffer();
	virtual void createDescriptorSet();
	virtual void createCommandBuffers();

	virtual void initFramebuffers();
	virtual void initDepthResources();

	virtual void initGraphicsPipeline();
};