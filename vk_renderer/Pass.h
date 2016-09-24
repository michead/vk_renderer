#pragma once

#include "VkEngine.h"


#define OPAQUE_BLACK_CLEAR	{ 0, 0, 0, 1 }
#define DEPTH_STENCIL_CLEAR	{ 1, 0 }


struct Texture;


struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};


class Pass {
	friend class VkEngine;

public:
	Pass(std::string vsPath, std::string fsPath) : vsPath(vsPath), fsPath(fsPath) { }
	Pass(std::string vsPath, std::string gsPath, std::string fsPath) : vsPath(vsPath), gsPath(gsPath), fsPath(fsPath) { }
	~Pass() { }

	virtual void init();
	virtual void updateData() { /*NOP*/ }
	
	virtual VkCommandBuffer getCurrentCommandBuffer() = 0;

protected:
	std::string vsPath;
	std::string gsPath;
	std::string fsPath;

	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
	std::vector<VkCommandBuffer> commandBuffers;
	VkImage depthImage;
	VkImageView depthImageView;
	VkDeviceMemory depthImageMemory;
	VkBuffer uniformStagingBuffer;
	VkDeviceMemory uniformStagingBufferMemory;
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;
	VkDescriptorSet descriptorSet;

	virtual void initUniformBuffer();
	virtual void initMeshBuffers();
	virtual void initTextures();
	virtual void initDepthResources();
	virtual void initFramebuffers() { }

	virtual void initAttachments() = 0;
	virtual void initCommandBuffers() = 0;
	virtual void initDescriptorSet() = 0;
	virtual void initGraphicsPipeline() = 0;

private:
	std::vector<VkFramebuffer> framebuffers;
};