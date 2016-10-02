#pragma once

#include <vector>

#include "Mesh.h"
#include "Pass.h"


class ShadowPass : public Pass {
	using Pass::Pass;

public:
	ShadowPass(std::string vs, std::string fs, size_t numLights) : Pass(vs, fs), numLights(numLights) { }
	~ShadowPass() { }

	size_t getNumLights() const { return numLights; }
	VkSemaphore getSemaphoreAt(size_t index) const { return semaphores[index]; }
	VkCommandBuffer getCmdBufferAt(size_t index) const { return commandBuffers[index]; }

	virtual void updateBufferData() override;

private:
	VkRenderPass renderPass;
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkFramebuffer> framebuffers;
	std::vector<GBufferAttachment> attachments;
	std::vector<VkSemaphore> semaphores;
	VkDescriptorSet descriptorSet;

	VkBuffer cameraUniformStagingBuffer;
	VkDeviceMemory cameraUniformStagingBufferMemory;
	VkBuffer cameraUniformBuffer;
	VkDeviceMemory cameraUniformBufferMemory;
	VkBuffer meshUniformStagingBuffer;
	VkDeviceMemory meshUniformStagingBufferMemory;
	VkBuffer meshUniformBuffer;
	VkDeviceMemory meshUniformBufferMemory;

	size_t numLights;

	virtual void initAttachments() override;
	virtual void initCommandBuffers() override;
	virtual void initDescriptorSets() override;
	virtual void initDescriptorSetLayout() override;
	virtual void initGraphicsPipeline() override;
	virtual void initSemaphores() override;
	virtual void initUniformBuffer() override;

	void loadMeshUniforms(const Mesh* mesh);
};