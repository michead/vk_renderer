#pragma once

#include "Pass.h"
#include "Quad.h"
#include "Scene.h"


struct LightingPassUniformBufferObject {
	Light lights[MAX_NUM_LIGHTS];
	uint16_t numLights;
	glm::vec3 cameraPos;
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
	LightingPassUniformBufferObject ubo;

	virtual void initAttachments() override;
	virtual void initCommandBuffers() override;
	virtual void initDescriptorSet() override;
	virtual void initDescriptorSetLayout() override;
	virtual void initGraphicsPipeline() override;
	virtual void initUniformBuffer() override;
};