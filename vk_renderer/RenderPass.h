#pragma once

#include "VkApp.h"
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

	VK_VEC_WRAP(VkFramebuffer) swapchainFramebuffers;
	
	VK_WRAP(VkImage) depthImage { VkApp::getDevice(), vkDestroyImage };
	VK_WRAP(VkImageView) depthImageView { VkApp::getDevice(), vkDestroyImageView };
	VK_WRAP(VkDeviceMemory) depthImageMemory { VkApp::getDevice(), vkFreeMemory };

	VK_WRAP(VkRenderPass) renderPass { VkApp::getDevice(), vkDestroyRenderPass };
	VK_WRAP(VkPipelineLayout) pipelineLayout { VkApp::getDevice(), vkDestroyPipelineLayout };
	VK_WRAP(VkPipeline) graphicsPipeline { VkApp::getDevice(), vkDestroyPipeline };

	std::vector<VkCommandBuffer> commandBuffers;

	VK_WRAP(VkBuffer) uniformStagingBuffer{ VkApp::getDevice(), vkDestroyBuffer };
	VK_WRAP(VkDeviceMemory) uniformStagingBufferMemory{ VkApp::getDevice(), vkFreeMemory };
	VK_WRAP(VkBuffer) uniformBuffer{ VkApp::getDevice(), vkDestroyBuffer };
	VK_WRAP(VkDeviceMemory) uniformBufferMemory{ VkApp::getDevice(), vkFreeMemory };

	std::vector<VkDescriptorSetLayout> layouts;
	VkDescriptorSet descriptorSet;

	static std::vector<VkDescriptorSetLayout>& getDescriptorSetLayouts(std::vector<Texture*> textures);

	virtual void initImageViews();
	virtual void initDescriptorSetLayout();
	virtual void initGraphicsPipeline();
	virtual void loadModels();

	virtual void createUniformBuffer();
	virtual void createDescriptorSet();
	virtual void createCommandBuffers();

	virtual void initFramebuffers();
	virtual void initCommandPool();
	virtual void initTextures();
	virtual void initDepthResources();

	virtual void initGraphicsPipeline();
};