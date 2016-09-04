#pragma once

#include "VkEngine.h"

#define SHADER_MAIN "main"


struct Texture;


class RenderPass {
public:
	RenderPass(std::string vsPath, std::string gsPath, std::string fsPath) : vsPath(vsPath), gsPath(gsPath), fsPath(fsPath) { }
	~RenderPass() { cleanup(); }

	virtual void init();
	virtual VkResult run();
	virtual void updateData();

protected:
	std::string vsPath;
	std::string gsPath;
	std::string fsPath;

	std::vector<VkFramebuffer> swapchainFramebuffers;
	
	VkImage depthImage;
	VkImageView depthImageView;
	VkDeviceMemory depthImageMemory;

	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	std::vector<VkCommandBuffer> commandBuffers;

	VkBuffer uniformStagingBuffer;
	VkDeviceMemory uniformStagingBufferMemory;
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;

	std::vector<VkDescriptorSetLayout> layouts;
	VkDescriptorSet descriptorSet;

	static void getDescriptorSetLayouts(std::vector<Texture*>& textures, std::vector<VkDescriptorSetLayout>&);

	virtual void createUniformBuffer();
	virtual void createDescriptorSet();
	virtual void createCommandBuffers();

	virtual void initFramebuffers();
	virtual void initDepthResources();
	virtual void initGraphicsPipeline();

	virtual void cleanup();
};