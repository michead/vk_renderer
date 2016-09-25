#pragma once

#include "VkEngine.h"
#include "GBuffer.h"


#define OPAQUE_BLACK_CLEAR	{ 0, 0, 0, 1 }
#define DEPTH_STENCIL_CLEAR	{ 1, 0 }


struct Texture;


class Pass {
	friend class VkEngine;

public:
	Pass(std::string vsPath, std::string fsPath) : vsPath(vsPath), fsPath(fsPath) { }
	Pass(std::string vsPath, std::string gsPath, std::string fsPath) : vsPath(vsPath), gsPath(gsPath), fsPath(fsPath) { }
	~Pass() { }

	virtual void init();
	virtual void updateBufferData() { /*NOP*/ }
	
	virtual VkCommandBuffer getCurrentCommandBuffer() const = 0;
	virtual GBuffer* getGBuffer() { return nullptr; }

protected:
	std::string vsPath;
	std::string gsPath;
	std::string fsPath;

	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
	VkImage depthImage;
	VkImageView depthImageView;
	VkDeviceMemory depthImageMemory;
	VkBuffer uniformStagingBuffer;
	VkDeviceMemory uniformStagingBufferMemory;
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;
	VkDescriptorSet descriptorSet;
	VkDescriptorSetLayout descriptorSetLayout;

	virtual void initMeshBuffers();
	virtual void initTextures();
	virtual void initDepthResources();
	virtual void initFramebuffers() { }

	virtual void initAttachments() = 0;
	virtual void initCommandBuffers() = 0;
	virtual void initDescriptorSet() = 0;
	virtual void initDescriptorSetLayout() = 0;
	virtual void initGraphicsPipeline() = 0;
	virtual void initUniformBuffer() = 0;

private:
	VkRenderPass renderPass;
	std::vector<VkFramebuffer> framebuffers;
	std::vector<VkCommandBuffer> commandBuffers;
};