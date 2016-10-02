#pragma once

#include "Pass.h"
#include "GBuffer.h"


#define ALBEDO_BINDING		2
#define NORMAL_BINDING		3


class Mesh;
struct Material;

struct GPMeshUniformBufferObject {
	glm::mat4 model;
};

struct GPCameraUniformBufferObject {
	glm::mat4 view;
	glm::mat4 proj;
};

struct GPMaterialUniformBufferObject {
	// Foruth component is useless, but padding would have been added anyway, so...
	glm::vec4	kd;
	glm::vec4	ks;
	float		rs;
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
	VkBuffer meshUniformStagingBuffer;
	VkDeviceMemory meshUniformStagingBufferMemory;
	VkBuffer meshUniformBuffer;
	VkDeviceMemory meshUniformBufferMemory;
	VkBuffer materialUniformStagingBuffer;
	VkDeviceMemory materialUniformStagingBufferMemory;
	VkBuffer materialUniformBuffer;
	VkDeviceMemory materialUniformBufferMemory;

	virtual void initAttachments() override;
	virtual void initCommandBuffers() override;
	virtual void initDescriptorSets() override;
	virtual void initDescriptorSetLayout() override;
	virtual void initGraphicsPipeline() override;
	virtual void initUniformBuffer() override;

	void loadMaterial(const Material* material);
	void loadMeshUniforms(const Mesh* mesh);

	int16_t loadedMaterial = -1;
};