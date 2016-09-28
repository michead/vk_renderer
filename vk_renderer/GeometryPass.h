#pragma once

#include "Pass.h"
#include "GBuffer.h"


#define ALBEDO_BINDING		1
#define NORMAL_BINDING		2
#define SPECULAR_BINDING	3


struct Material;


struct GPCameraUniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

struct GPMaterialUniformBufferObject {
	glm::vec3	kd;
	glm::vec3	ks;
	bool		kdMap;
	bool		ksMap;
	bool		normalMap;
	float		ns;
	float		opacity;
	float		translucency;
	float		subsurfWidth;
};


class GeometryPass : public Pass {
	using Pass::Pass;

public:
	virtual void initBufferData() override;
	virtual void updateBufferData() override;

	virtual VkCommandBuffer getCurrentCommandBuffer() const override { return commandBuffer; }
	virtual GBuffer* getGBuffer() override { return &gBuffer; }

private:
	GBuffer gBuffer;
	VkCommandBuffer commandBuffer;
	VkBuffer cameraUniformStagingBuffer;
	VkDeviceMemory cameraUniformStagingBufferMemory;
	VkBuffer cameraUniformBuffer;
	VkDeviceMemory cameraUniformBufferMemory;
	VkBuffer materialUniformStagingBuffer;
	VkDeviceMemory materialUniformStagingBufferMemory;
	VkBuffer materialUniformBuffer;
	VkDeviceMemory materialUniformBufferMemory;

	int16_t lastMaterialId = -1;

	virtual void initAttachments() override;
	virtual void initCommandBuffers() override;
	virtual void initDescriptorSets() override;
	virtual void initDescriptorSetLayout() override;
	virtual void initGraphicsPipeline() override;
	virtual void initUniformBuffer() override;

	void updateMaterial(Material* material);
};