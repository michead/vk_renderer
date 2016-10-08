#pragma once

#include <vector>

#include "Light.h"
#include "Mesh.h"
#include "Pass.h"
#include "Scene.h"


#define DEPTH_BIAS_CONSTANT	1.25f
#define DEPTH_BIAS_SLOPE	1.75f


class ShadowPass : public Pass {
	using Pass::Pass;

public:
	virtual void initBufferData() override;
	virtual void updateBufferData() override;

	ShadowPass(std::string vs, std::string fs) : Pass(vs, fs) 
	{ lights = VkEngine::getEngine().getScene()->getLights(); attachments.resize(lights.size()); }
	~ShadowPass() { }

	size_t getNumLights() const { return lights.size(); }
	VkSemaphore getSemaphoreAt(size_t index) const { return semaphores[index]; }
	VkCommandBuffer getCmdBufferAt(size_t index) const { return commandBuffers[index]; }
	GBufferAttachment* getMaps() { return attachments.data(); }

private:
	VkRenderPass renderPass;
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkFramebuffer> framebuffers;
	std::vector<GBufferAttachment> attachments;
	std::vector<VkSemaphore> semaphores;

	std::vector<VkBuffer> cameraUniformStagingBuffers;
	std::vector<VkDeviceMemory> cameraUniformStagingBufferMemoryList;
	std::vector<VkBuffer> cameraUniformBuffers;
	std::vector<VkDeviceMemory> cameraUniformBufferMemoryList;
	VkBuffer meshUniformStagingBuffer;
	VkDeviceMemory meshUniformStagingBufferMemory;
	VkBuffer meshUniformBuffer;
	VkDeviceMemory meshUniformBufferMemory;

	std::vector<Light*> lights;

	virtual void initAttachments() override;
	virtual void initCommandBuffers() override;
	virtual void initDescriptorSets() override;
	virtual void initDescriptorSetLayout() override;
	virtual void initGraphicsPipeline() override;
	virtual void initSemaphores() override;
	virtual void initUniformBuffer() override;

	void loadMeshUniforms(const Mesh* mesh);
	void loadLightUniforms(size_t lightIndex);
};