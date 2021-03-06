#pragma once

#include "Pass.h"
#include "Quad.h"
#include "Scene.h"


struct LPShaderLight{
	glm::vec4 pos;
	glm::vec4 ke;
	glm::mat4 mat;
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
	LightingPass(std::string vsPath, std::string fsPath, GBuffer* prevPassGBuffer, 
		size_t numShadowMaps, GBufferAttachment* shadowMaps, GBufferAttachment* aoMap, bool isFinalPass) :
		Pass(vsPath, fsPath), prevPassGBuffer(prevPassGBuffer), numShadowMaps(numShadowMaps), 
		shadowMaps(shadowMaps), aoMap(aoMap)
		{ quad = new Quad(); }
	~LightingPass() { delete quad; }

	VkCommandBuffer getCurrentCmdBuffer() const { return commandBuffers[0]; }
	GBufferAttachment* getDiffuseAttachment() { return &diffuseAttachment; }
	GBufferAttachment* getSpecularAttachment() { return &specularAttachment; }

	virtual void initBufferData() override;
	virtual void updateBufferData() override;

private:
	bool isFinalPass;

	VkRenderPass renderPass;
	std::vector<VkCommandBuffer> commandBuffers;
	VkFramebuffer framebuffer;

	Quad* quad;
	GBuffer* prevPassGBuffer;
	size_t numShadowMaps;
	GBufferAttachment* shadowMaps;
	GBufferAttachment* aoMap;
	GBufferAttachment diffuseAttachment;
	GBufferAttachment specularAttachment;
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