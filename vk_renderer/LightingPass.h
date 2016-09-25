#pragma once

#include "Pass.h"
#include "Quad.h"


class LightingPass : public Pass {
public:
	LightingPass(std::string vsPath, std::string fsPath, GBuffer* prevPassGBuffer) :
		Pass(vsPath, fsPath), prevPassGBuffer(prevPassGBuffer) { quad = new Quad(); }
	~LightingPass() { delete quad; }

	virtual VkCommandBuffer getCurrentCommandBuffer() const override
	{ 
		return commandBuffers[VkEngine::getEngine().getSwapchainImageIndex()]; 
	}

private:
	VkRenderPass renderPass;
	std::vector<VkCommandBuffer> commandBuffers;

	Quad* quad;
	GBuffer* prevPassGBuffer;
	VkAttachmentDescription colorAttachment;

	virtual void initAttachments() override;
	virtual void initCommandBuffers() override;
	virtual void initDescriptorSet() override;
	virtual void initGraphicsPipeline() override;
};