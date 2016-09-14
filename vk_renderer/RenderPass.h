#pragma once

#include "VkEngine.h"


struct Texture;


struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};



class RenderPass {
	friend class VkEngine;

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
	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	std::vector<VkCommandBuffer> commandBuffers;
	VkImage depthImage;
	VkImageView depthImageView;
	VkDeviceMemory depthImageMemory;
	VkBuffer uniformStagingBuffer;
	VkDeviceMemory uniformStagingBufferMemory;
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;

	virtual void initUniformBuffer();
	virtual void initDescriptorSet();
	virtual void initDescriptorSetLayout();
	virtual void initCommandBuffers();

	virtual void initAttachments();
	virtual void initFramebuffers();
	virtual void initMeshBuffers();
	virtual void initTextures();
	virtual void initDepthResources();
	virtual void initGraphicsPipeline();

	virtual void deleteSwapchainFramebuffers();
	virtual void deleteUniformBuffers();
	virtual void deleteDepthResources();
	virtual void deleteDescriptorSetLayout();
	virtual void deleteRenderPass();
	virtual void deleteGraphicsPipeline();
};