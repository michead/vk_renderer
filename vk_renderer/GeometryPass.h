#pragma once

#include "Pass.h"
#include "GBuffer.h"


struct GeometryPassUniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};


class GeometryPass : public Pass {
	using Pass::Pass;

public:
	virtual void updateBufferData() override;

	virtual VkCommandBuffer getCurrentCommandBuffer() const override { return commandBuffer; }
	virtual GBuffer* getGBuffer() override { return &gBuffer; }

private:
	GBuffer gBuffer;
	VkCommandBuffer commandBuffer;

	virtual void initAttachments() override;
	virtual void initCommandBuffers() override;
	virtual void initDescriptorSet() override;
	virtual void initDescriptorSetLayout() override;
	virtual void initGraphicsPipeline() override;
	virtual void initUniformBuffer() override;
};