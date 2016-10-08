#pragma once

#include "Pass.h"
#include "Quad.h"
#include "Scene.h"


class MergePass : public Pass {
public:
	MergePass(std::string vsPath, std::string fsPath, GBufferAttachment* diffuseAttachment, 
		GBufferAttachment* specularAttachment) :
		vsPath(vsPath), fsPath(fsPath), diffuseAttachment(diffuseAttachment), 
		specularAttachment(specularAttachment) { quad = new Quad(); }
	~MergePass() { delete quad; }

	VkCommandBuffer getCurrentCmdBuffer() const { return commandBuffers[VkEngine::getEngine().getSwapchainImageIndex()]; }

private:
	std::string vsPath, fsPath;

	VkRenderPass renderPass;
	std::vector<VkCommandBuffer> commandBuffers;

	Quad* quad;
	GBuffer* prevPassGBuffer;
	size_t numShadowMaps;
	GBufferAttachment* diffuseAttachment;
	GBufferAttachment* specularAttachment;
	VkAttachmentDescription colorAttachment;

	virtual void initAttachments() override;
	virtual void initCommandBuffers() override;
	virtual void initDescriptorSets() override;
	virtual void initDescriptorSetLayout() override;
	virtual void initGraphicsPipeline() override;
};