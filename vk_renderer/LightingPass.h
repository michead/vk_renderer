#pragma once

#include "Pass.h"
#include "Quad.h"
#include "Scene.h"


struct LPLightsUniformBufferObject {
	glm::vec4 positions[MAX_NUM_LIGHTS];
	glm::vec4 intensities[MAX_NUM_LIGHTS];
	int count;
};

struct LPCameraUniformBufferObject {
	glm::vec4 position;
};

struct LPSceneUniformBufferObject {
	glm::vec4 ambient;
};


class LightingPass : public Pass {
public:
	LightingPass(std::string vsPath, std::string fsPath, GBuffer* prevPassGBuffer, size_t numShadowMaps, GBufferAttachment* shadowMaps) :
		Pass(vsPath, fsPath), prevPassGBuffer(prevPassGBuffer), numShadowMaps(numShadowMaps), shadowMaps(shadowMaps) { quad = new Quad(); }
	~LightingPass() { delete quad; }

	VkCommandBuffer getCurrentCmdBuffer() const { return commandBuffers[VkEngine::getEngine().getSwapchainImageIndex()]; }

	virtual void initBufferData() override;
	virtual void updateBufferData() override;

private:
	VkRenderPass renderPass;
	std::vector<VkCommandBuffer> commandBuffers;

	Quad* quad;
	GBuffer* prevPassGBuffer;
	size_t numShadowMaps;
	GBufferAttachment* shadowMaps;
	VkAttachmentDescription colorAttachment;
	LPLightsUniformBufferObject lightsUBO;
	LPCameraUniformBufferObject cameraUBO;
	LPSceneUniformBufferObject sceneUBO;
	VkBuffer lightsUniformStagingBuffer;
	VkDeviceMemory lightsUniformStagingBufferMemory;
	VkBuffer lightsUniformBuffer;
	VkDeviceMemory lightsUniformBufferMemory;
	VkBuffer cameraUniformStagingBuffer;
	VkDeviceMemory cameraUniformStagingBufferMemory;
	VkBuffer cameraUniformBuffer;
	VkDeviceMemory cameraUniformBufferMemory;
	VkBuffer sceneUniformStagingBuffer;
	VkDeviceMemory sceneUniformStagingBufferMemory;
	VkBuffer sceneUniformBuffer;
	VkDeviceMemory sceneUniformBufferMemory;

	virtual void initAttachments() override;
	virtual void initCommandBuffers() override;
	virtual void initDescriptorSets() override;
	virtual void initDescriptorSetLayout() override;
	virtual void initGraphicsPipeline() override;
	virtual void initUniformBuffer() override;
};