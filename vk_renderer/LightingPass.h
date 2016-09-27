#pragma once

#include "Pass.h"
#include "Quad.h"
#include "Scene.h"


struct LPLightsUniformBufferObject {
	int count;
	glm::vec3 positions[MAX_NUM_LIGHTS];
	glm::vec3 intensities[MAX_NUM_LIGHTS];
};

struct LPCameraUniformBufferObject {
	glm::vec3 position;
};


class LightingPass : public Pass {
public:
	LightingPass(std::string vsPath, std::string fsPath, GBuffer* prevPassGBuffer) :
		Pass(vsPath, fsPath), prevPassGBuffer(prevPassGBuffer) { quad = new Quad(); }
	~LightingPass() { delete quad; }

	virtual VkCommandBuffer getCurrentCommandBuffer() const override
	{ 
		return commandBuffers[VkEngine::getEngine().getSwapchainImageIndex()]; 
	}

	virtual void initBufferData() override;
	virtual void updateBufferData() override;

private:
	VkRenderPass renderPass;
	std::vector<VkCommandBuffer> commandBuffers;

	Quad* quad;
	GBuffer* prevPassGBuffer;
	VkAttachmentDescription colorAttachment;
	LPLightsUniformBufferObject lightsUBO;
	LPCameraUniformBufferObject cameraUBO;
	VkBuffer lightsUniformStagingBuffer;
	VkDeviceMemory lightsUniformStagingBufferMemory;
	VkBuffer lightsUniformBuffer;
	VkDeviceMemory lightsUniformBufferMemory;
	VkBuffer cameraUniformStagingBuffer;
	VkDeviceMemory cameraUniformStagingBufferMemory;
	VkBuffer cameraUniformBuffer;
	VkDeviceMemory cameraUniformBufferMemory;

	virtual void initAttachments() override;
	virtual void initCommandBuffers() override;
	virtual void initDescriptorSets() override;
	virtual void initDescriptorSetLayout() override;
	virtual void initGraphicsPipeline() override;
	virtual void initUniformBuffer() override;
};