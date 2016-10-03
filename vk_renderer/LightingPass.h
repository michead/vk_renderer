#pragma once

#include "Pass.h"
#include "Quad.h"
#include "Scene.h"


struct LPShaderLight{
	glm::vec4 pos;
	glm::vec4 ke;
};

struct LPCameraUniformBufferObject {
	glm::vec4 position;
};

struct LPSceneUniformBufferObject {
	glm::vec4 ka;
	LPShaderLight lights[MAX_NUM_LIGHTS];
	int numLights;
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
	LPCameraUniformBufferObject cameraUBO;
	LPSceneUniformBufferObject sceneUBO;
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