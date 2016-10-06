#pragma once

#include "Pass.h"
#include "Quad.h"
#include "Scene.h"


#define KERNEL_SIZE 32
#define NOISE_SIZE	4


struct SSAOPCameraUniformBufferObject {
	glm::vec4 noiseScale;
	glm::mat4 proj;
};

struct SSAOPKernelUniformBufferObject {
	glm::vec4 sampleKernel[KERNEL_SIZE];
};


class SSAOPass : public Pass {
public:
	SSAOPass(std::string vsPath, std::string fsPath, GBuffer* gBuffer) :
		Pass(vsPath, fsPath), gBuffer(gBuffer)
	{
		quad = new Quad();

		computeNoiseScale();
		computeKernel();
		computeNoiseTexels();
		loadNoiseTexture();
	}
	~SSAOPass() { delete quad; delete noiseTexture; }

	VkCommandBuffer getMainPassCmdBuffer() const { return commandBuffers[0]; }
	VkCommandBuffer getBlurPassCmdBuffer() const { return commandBuffers[1]; }

	virtual void initBufferData() override;
	virtual void updateBufferData() override;

private:
	VkRenderPass renderPass;
	VkSemaphore mainPassSemaphore;
	std::array<VkCommandBuffer, 2> commandBuffers;
	std::array<VkFramebuffer, 2> framebuffers;
	std::array<VkDescriptorSet, 2> descriptorSets;
	std::array<VkPipeline, 2> pipelines;
	std::array<VkPipelineLayout, 2> pipelineLayouts;
	VkDescriptorSetLayout mainPassDescriptorSetLayout;
	VkDescriptorSetLayout blurPassDescriptorSetLayout;
	GBuffer* gBuffer;

	glm::vec4 sampleKernel[KERNEL_SIZE];
	std::vector<glm::vec4> noiseTexels;
	Texture* noiseTexture;
	glm::vec2 noiseScale;

	Quad* quad;
	GBufferAttachment aoAttachment;
	GBufferAttachment blurredAOAttachment;
	VkAttachmentDescription colorAttachment;
	SSAOPCameraUniformBufferObject cameraUBO;
	SSAOPKernelUniformBufferObject kernelUBO;
	VkBuffer cameraUniformStagingBuffer;
	VkDeviceMemory cameraUniformStagingBufferMemory;
	VkBuffer cameraUniformBuffer;
	VkDeviceMemory cameraUniformBufferMemory;
	VkBuffer kernelUniformStagingBuffer;
	VkDeviceMemory kernelUniformStagingBufferMemory;
	VkBuffer kernelUniformBuffer;
	VkDeviceMemory kernelUniformBufferMemory;

	virtual void initAttachments() override;
	virtual void initCommandBuffers() override;
	virtual void initDescriptorSets() override;
	virtual void initDescriptorSetLayout() override;
	virtual void initGraphicsPipeline() override;
	virtual void initUniformBuffer() override;
	virtual void initSemaphores() override;

	void computeNoiseScale();
	void computeKernel();
	void computeNoiseTexels();
	void loadNoiseTexture();
	void loadKernelUniforms();
	void loadCameraUniforms();
};