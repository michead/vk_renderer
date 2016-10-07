#pragma once

#include "Pass.h"
#include "Quad.h"
#include "Scene.h"


class MergePass : public Pass {
public:
	MergePass(std::string vsPath, std::string fsPath, GBufferAttachment* diffuseAttachment, 
		GBufferAttachment* specularAttachment, bool isFInalPass) :
		vsPath(vsPath), fsPath(fsPath), diffuseAttachment(diffuseAttachment), 
		specularAttachment(specularAttachment), isFinalPass(isFInalPass) { }
	~MergePass() { delete quad; }

	GBufferAttachment* getColorAttachment() { return &attachment; }
	VkCommandBuffer getCurrentCmdBuffer() const
	{
		return isFinalPass ? commandBuffers[VkEngine::getEngine().getSwapchainImageIndex()] : commandBuffers[0];
	}

private:
	std::string vsPath;
	std::string fsPath;

	VkRenderPass renderPass;
	VkSemaphore mainPassSemaphore;
	std::vector<VkCommandBuffer> commandBuffers;
	VkFramebuffer framebuffer;
	GBufferAttachment* diffuseAttachment;
	GBufferAttachment* specularAttachment;

	Quad* quad;
	GBufferAttachment attachment;
	bool isFinalPass;

	virtual void initAttachments() override;
	virtual void initCommandBuffers() override;
	virtual void initDescriptorSets() override;
	virtual void initDescriptorSetLayout() override;
	virtual void initGraphicsPipeline() override;
};