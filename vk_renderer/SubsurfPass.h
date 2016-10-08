#pragma once

#include "Pass.h"
#include "Quad.h"
#include "Scene.h"

#define SS_NUM_SAMPLES	17
#define SS_STRENGTH		{	.48f,	.41f,	.28f	}
#define SS_FALLOFF		{	1.f,	.37f,	.3f		}


struct SSSPCameraUniformBufferObject {
	float fovy;
};

struct SSSPInstanceUniformBufferObject {
	glm::vec4 kernel[SS_NUM_SAMPLES];
	glm::vec2 blurDirection;
};


class SubsurfPass : public Pass {
public:
	SubsurfPass(std::string vsPath, std::string fsPath, glm::vec2 blurDirection, GBuffer* gBuffer) :
		vsPath(vsPath), fsPath(fsPath), gBuffer(gBuffer), blurDirection(blurDirection) 
	{ quad = new Quad(); computeKernel(SS_STRENGTH, SS_FALLOFF); inColorAttachment = &gBuffer->attachments[GBUFFER_COLOR_ATTACH_ID]; }
	SubsurfPass(std::string vsPath, std::string fsPath, glm::vec2 blurDirection, GBuffer* gBuffer, GBufferAttachment* inColorAttachment) :
		vsPath(vsPath), fsPath(fsPath), gBuffer(gBuffer), blurDirection(blurDirection), inColorAttachment(inColorAttachment)
	{ quad = new Quad(); computeKernel(SS_STRENGTH, SS_FALLOFF); }
	~SubsurfPass() { delete quad; }

	VkCommandBuffer getCurrentCmdBuffer() const { return commandBuffer; }
	GBufferAttachment* getColorAttachment() { return &attachment; }

	virtual void initBufferData() override;
	virtual void updateBufferData() override;

private:
	std::string vsPath;
	std::string fsPath;

	VkRenderPass renderPass;
	VkSemaphore mainPassSemaphore;
	VkCommandBuffer commandBuffer;
	VkFramebuffer framebuffer;

	glm::vec2 blurDirection;
	glm::vec4 kernel[SS_NUM_SAMPLES];

	Quad* quad;
	GBufferAttachment attachment;
	GBuffer* gBuffer;
	GBufferAttachment* inColorAttachment;
	SSSPCameraUniformBufferObject cameraUBO;
	SSSPInstanceUniformBufferObject instanceUBO;
	VkBuffer cameraUniformStagingBuffer;
	VkDeviceMemory cameraUniformStagingBufferMemory;
	VkBuffer cameraUniformBuffer;
	VkDeviceMemory cameraUniformBufferMemory;
	VkBuffer instanceUniformStagingBuffer;
	VkDeviceMemory instanceUniformStagingBufferMemory;
	VkBuffer instanceUniformBuffer;
	VkDeviceMemory instanceUniformBufferMemory;

	virtual void initAttachments() override;
	virtual void initCommandBuffers() override;
	virtual void initDescriptorSets() override;
	virtual void initDescriptorSetLayout() override;
	virtual void initGraphicsPipeline() override;
	virtual void initUniformBuffer() override;

	void computeKernel(glm::vec3 strength, glm::vec3 falloff);
	void loadCameraUniforms();
	void loadInstanceUniforms();

	static glm::vec3 profile(glm::vec3 falloff, float r)
	{
		return  //	0.233f * gaussian(falloff, 0.0064f, r) +
					0.100f * gaussian(falloff, 0.0484f, r) +
					0.118f * gaussian(falloff, 0.187f, r) +
					0.113f * gaussian(falloff, 0.567f, r) +
					0.358f * gaussian(falloff, 1.99f, r) +
					0.078f * gaussian(falloff, 7.41f, r);
	}
};
