#pragma once

#include "Pass.h"
#include "Quad.h"


class FinalPass : public Pass {
public:
	FinalPass(std::string vsPath, std::string fsPath) : Pass(vsPath, fsPath) { quad = new Quad(); }
	~FinalPass() { delete quad; }

	virtual VkCommandBuffer getCurrentCommandBuffer() override
	{ 
		return commandBuffers[VkEngine::getEngine().getSwapchainImageIndex()]; 
	}

private:
	Quad* quad;
	VkAttachmentDescription colorAttachment;

	virtual void initAttachments() override;
	virtual void initCommandBuffers() override;
	virtual void initFramebuffers() override;
	virtual void initDescriptorSet() override;
};