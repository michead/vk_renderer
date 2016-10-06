#include "LightingPass.h"

#include <array>

#include "Camera.h"
#include "VkPool.h"


void LightingPass::initAttachments()
{
	colorAttachment = {};
	colorAttachment.format = VkEngine::getEngine().getSwapchainFormat();
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subPass = {};
	subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subPass.colorAttachmentCount = 1;
	subPass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subPass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	renderPass = VkEngine::getEngine().getPool()->createRenderPass(renderPassInfo);
}

void LightingPass::initCommandBuffers()
{
	commandBuffers.resize(VkEngine::getEngine().getSwapchainImageViews().size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = VkEngine::getEngine().getCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = commandBuffers.size();

	VK_CHECK(vkAllocateCommandBuffers(VkEngine::getEngine().getDevice(), &allocInfo, commandBuffers.data()));

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = TRANSPARENT_BLACK_CLEAR;
	clearValues[1].depthStencil = DEPTH_STENCIL_CLEAR;

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = VkEngine::getEngine().getSwapchainExtent();
	renderPassInfo.clearValueCount = clearValues.size();
	renderPassInfo.pClearValues = clearValues.data();

	std::vector<VkFramebuffer> framebuffers = VkEngine::getEngine().getSwapchainFramebuffers();

	for (size_t i = 0; i < commandBuffers.size(); i++)
	{
		vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

		renderPassInfo.framebuffer = framebuffers[i];

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		VkBuffer vertexBuffers[] = { quad->getVertexBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffers[i], quad->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(
			commandBuffers[i],
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			1,
			&descriptorSets[0],
			0,
			nullptr);

		vkCmdDrawIndexed(commandBuffers[i], quad->indices.size(), 1, 0, 0, 1);

		vkCmdEndRenderPass(commandBuffers[i]);

		VK_CHECK(vkEndCommandBuffer(commandBuffers[i]));
	}
}

void LightingPass::initDescriptorSets()
{
	descriptorSets.resize(1);

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = VkEngine::getEngine().getDescriptorPool();
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout;

	VK_CHECK(vkAllocateDescriptorSets(VkEngine::getEngine().getDevice(), &allocInfo, &descriptorSets[0]));

	uint32_t bindingIndex = 0;

	std::vector<VkWriteDescriptorSet> descriptorWrites;

	VkDescriptorImageInfo colorImageInfo = {};
	colorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	colorImageInfo.imageView = prevPassGBuffer->attachments[GBUFFER_COLOR_ATTACH_ID].imageView;
	colorImageInfo.sampler = prevPassGBuffer->attachments[GBUFFER_COLOR_ATTACH_ID].imageSampler;

	VkWriteDescriptorSet colorDescriptorSet = {};
	colorDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	colorDescriptorSet.dstSet = descriptorSets[0];
	colorDescriptorSet.dstBinding = bindingIndex++;
	colorDescriptorSet.dstArrayElement = 0;
	colorDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	colorDescriptorSet.descriptorCount = 1;
	colorDescriptorSet.pImageInfo = &colorImageInfo;

	descriptorWrites.push_back(colorDescriptorSet);

	VkDescriptorImageInfo positionImageInfo = {};
	positionImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	positionImageInfo.imageView = prevPassGBuffer->attachments[GBUFFER_POSITION_ATTACH_ID].imageView;
	positionImageInfo.sampler = prevPassGBuffer->attachments[GBUFFER_POSITION_ATTACH_ID].imageSampler;

	VkWriteDescriptorSet positionDescriptorSet = {};
	positionDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	positionDescriptorSet.dstSet = descriptorSets[0];
	positionDescriptorSet.dstBinding = bindingIndex++;
	positionDescriptorSet.dstArrayElement = 0;
	positionDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	positionDescriptorSet.descriptorCount = 1;
	positionDescriptorSet.pImageInfo = &positionImageInfo;

	descriptorWrites.push_back(positionDescriptorSet);

	VkDescriptorImageInfo normalImageInfo = {};
	normalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	normalImageInfo.imageView = prevPassGBuffer->attachments[GBUFFER_NORMAL_ATTACH_ID].imageView;
	normalImageInfo.sampler = prevPassGBuffer->attachments[GBUFFER_NORMAL_ATTACH_ID].imageSampler;

	VkWriteDescriptorSet normalDescriptorSet = {};
	normalDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	normalDescriptorSet.dstSet = descriptorSets[0];
	normalDescriptorSet.dstBinding = bindingIndex++;
	normalDescriptorSet.dstArrayElement = 0;
	normalDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	normalDescriptorSet.descriptorCount = 1;
	normalDescriptorSet.pImageInfo = &normalImageInfo;

	descriptorWrites.push_back(normalDescriptorSet);

	VkDescriptorImageInfo tangentImageInfo = {};
	tangentImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	tangentImageInfo.imageView = prevPassGBuffer->attachments[GBUFFER_TANGENT_ATTACH_ID].imageView;
	tangentImageInfo.sampler = prevPassGBuffer->attachments[GBUFFER_TANGENT_ATTACH_ID].imageSampler;

	VkWriteDescriptorSet tangentDescriptorSet = {};
	tangentDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	tangentDescriptorSet.dstSet = descriptorSets[0];
	tangentDescriptorSet.dstBinding = bindingIndex++;
	tangentDescriptorSet.dstArrayElement = 0;
	tangentDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	tangentDescriptorSet.descriptorCount = 1;
	tangentDescriptorSet.pImageInfo = &tangentImageInfo;

	descriptorWrites.push_back(tangentDescriptorSet);

	VkDescriptorImageInfo specularImageInfo = {};
	specularImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	specularImageInfo.imageView = prevPassGBuffer->attachments[GBUFFER_SPECULAR_ATTACH_ID].imageView;
	specularImageInfo.sampler = prevPassGBuffer->attachments[GBUFFER_SPECULAR_ATTACH_ID].imageSampler;

	VkWriteDescriptorSet specularDescriptorSet = {};
	specularDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	specularDescriptorSet.dstSet = descriptorSets[0];
	specularDescriptorSet.dstBinding = bindingIndex++;
	specularDescriptorSet.dstArrayElement = 0;
	specularDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	specularDescriptorSet.descriptorCount = 1;
	specularDescriptorSet.pImageInfo = &specularImageInfo;

	descriptorWrites.push_back(specularDescriptorSet);

	VkDescriptorImageInfo materialImageInfo = {};
	materialImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	materialImageInfo.imageView = prevPassGBuffer->attachments[GBUFFER_MATERIAL_ATTACH_ID].imageView;
	materialImageInfo.sampler = prevPassGBuffer->attachments[GBUFFER_MATERIAL_ATTACH_ID].imageSampler;

	VkWriteDescriptorSet materialDescriptorSet = {};
	materialDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	materialDescriptorSet.dstSet = descriptorSets[0];
	materialDescriptorSet.dstBinding = bindingIndex++;
	materialDescriptorSet.dstArrayElement = 0;
	materialDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	materialDescriptorSet.descriptorCount = 1;
	materialDescriptorSet.pImageInfo = &materialImageInfo;

	descriptorWrites.push_back(materialDescriptorSet);

	VkDescriptorImageInfo depthImageInfo = {};
	depthImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	depthImageInfo.imageView = prevPassGBuffer->attachments[GBUFFER_DEPTH_ATTACH_ID].imageView;
	depthImageInfo.sampler = prevPassGBuffer->attachments[GBUFFER_DEPTH_ATTACH_ID].imageSampler;

	VkWriteDescriptorSet depthDescriptorSet = {};
	depthDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	depthDescriptorSet.dstSet = descriptorSets[0];
	depthDescriptorSet.dstBinding = bindingIndex++;
	depthDescriptorSet.dstArrayElement = 0;
	depthDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	depthDescriptorSet.descriptorCount = 1;
	depthDescriptorSet.pImageInfo = &depthImageInfo;

	descriptorWrites.push_back(depthDescriptorSet);

	VkDescriptorBufferInfo cameraBufferInfo = {};
	cameraBufferInfo.buffer = cameraUniformBuffer;
	cameraBufferInfo.offset = 0;
	cameraBufferInfo.range = sizeof(LPCameraUniformBufferObject);

	VkWriteDescriptorSet cameraDescriptorSet = {};
	cameraDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	cameraDescriptorSet.dstSet = descriptorSets[0];
	cameraDescriptorSet.dstBinding = bindingIndex++;
	cameraDescriptorSet.dstArrayElement = 0;
	cameraDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	cameraDescriptorSet.descriptorCount = 1;
	cameraDescriptorSet.pBufferInfo = &cameraBufferInfo;

	descriptorWrites.push_back(cameraDescriptorSet);

	VkDescriptorBufferInfo sceneBufferInfo = {};
	sceneBufferInfo.buffer = sceneUniformBuffer;
	sceneBufferInfo.offset = 0;
	sceneBufferInfo.range = sizeof(LPSceneUniformBufferObject);

	VkWriteDescriptorSet sceneDescriptorSet = {};
	sceneDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	sceneDescriptorSet.dstSet = descriptorSets[0];
	sceneDescriptorSet.dstBinding = bindingIndex++;
	sceneDescriptorSet.dstArrayElement = 0;
	sceneDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	sceneDescriptorSet.descriptorCount = 1;
	sceneDescriptorSet.pBufferInfo = &sceneBufferInfo;

	descriptorWrites.push_back(sceneDescriptorSet);

	std::vector<VkDescriptorImageInfo> shadowImageInfos;
	for (size_t l = 0; l < numShadowMaps; l++)
	{
		VkDescriptorImageInfo shadowImageInfo = {};
		shadowImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		shadowImageInfo.imageView = shadowMaps[l].imageView;
		shadowImageInfo.sampler = shadowMaps[l].imageSampler;

		shadowImageInfos.push_back(shadowImageInfo);
	}

	VkWriteDescriptorSet shadowDescriptorSet = {};
	shadowDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	shadowDescriptorSet.dstSet = descriptorSets[0];
	shadowDescriptorSet.dstBinding = bindingIndex++;
	shadowDescriptorSet.dstArrayElement = 0;
	shadowDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	shadowDescriptorSet.descriptorCount = shadowImageInfos.size();
	shadowDescriptorSet.pImageInfo = shadowImageInfos.data();

	descriptorWrites.push_back(shadowDescriptorSet);

	VkDescriptorImageInfo aoImageInfo = {};
	aoImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	aoImageInfo.imageView = aoMap->imageView;
	aoImageInfo.sampler = aoMap->imageSampler;

	VkWriteDescriptorSet aoDescriptorSet = {};
	aoDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	aoDescriptorSet.dstSet = descriptorSets[0];
	aoDescriptorSet.dstBinding = bindingIndex++;
	aoDescriptorSet.dstArrayElement = 0;
	aoDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	aoDescriptorSet.descriptorCount = 1;
	aoDescriptorSet.pImageInfo = &aoImageInfo;

	descriptorWrites.push_back(aoDescriptorSet);

	vkUpdateDescriptorSets(VkEngine::getEngine().getDevice(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

void LightingPass::initGraphicsPipeline()
{
	std::vector<char> vs = readFile(vsPath);
	std::vector<char> fs = readFile(fsPath);
	std::vector<char> gs;

	if (!gsPath.empty())
	{
		gs = readFile(gsPath);
	}

	PipelineData pipelineData = VkEngine::getEngine().getPool()->createPipeline(
		renderPass,
		descriptorSetLayout,
		VkEngine::getEngine().getSwapchainExtent(),
		vs,
		fs,
		gs,
		1);

	pipeline = pipelineData.pipeline;
	pipelineLayout = pipelineData.pipelineLayout;
}

void LightingPass::initUniformBuffer()
{
	VkDeviceSize bufferSize = sizeof(LPCameraUniformBufferObject);

	std::vector<BufferData> bufferDataVec = VkEngine::getEngine().getPool()->createUniformBuffer(bufferSize, true);
	cameraUniformStagingBuffer = bufferDataVec[0].buffer;
	cameraUniformStagingBufferMemory = bufferDataVec[0].bufferMemory;
	cameraUniformBuffer = bufferDataVec[1].buffer;
	cameraUniformBufferMemory = bufferDataVec[1].bufferMemory;

	bufferSize = sizeof(LPSceneUniformBufferObject);

	bufferDataVec = VkEngine::getEngine().getPool()->createUniformBuffer(bufferSize, true);
	sceneUniformStagingBuffer = bufferDataVec[0].buffer;
	sceneUniformStagingBufferMemory = bufferDataVec[0].bufferMemory;
	sceneUniformBuffer = bufferDataVec[1].buffer;
	sceneUniformBufferMemory = bufferDataVec[1].bufferMemory;
}

void LightingPass::initDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	uint32_t bindingIndex = 0;

	VkDescriptorSetLayoutBinding colorSamplerLayoutBinding = {};
	colorSamplerLayoutBinding.binding = bindingIndex++;
	colorSamplerLayoutBinding.descriptorCount = 1;
	colorSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	colorSamplerLayoutBinding.pImmutableSamplers = nullptr;
	colorSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings.push_back(colorSamplerLayoutBinding);

	VkDescriptorSetLayoutBinding positionSamplerLayoutBinding = {};
	positionSamplerLayoutBinding.binding = bindingIndex++;
	positionSamplerLayoutBinding.descriptorCount = 1;
	positionSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	positionSamplerLayoutBinding.pImmutableSamplers = nullptr;
	positionSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	
	bindings.push_back(positionSamplerLayoutBinding);

	VkDescriptorSetLayoutBinding normalSamplerLayoutBinding = {};
	normalSamplerLayoutBinding.binding = bindingIndex++;
	normalSamplerLayoutBinding.descriptorCount = 1;
	normalSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	normalSamplerLayoutBinding.pImmutableSamplers = nullptr;
	normalSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings.push_back(normalSamplerLayoutBinding);

	VkDescriptorSetLayoutBinding tangentLayoutBinding = {};
	tangentLayoutBinding.binding = bindingIndex++;
	tangentLayoutBinding.descriptorCount = 1;
	tangentLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	tangentLayoutBinding.pImmutableSamplers = nullptr;
	tangentLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings.push_back(tangentLayoutBinding);

	VkDescriptorSetLayoutBinding specularLayoutBinding = {};
	specularLayoutBinding.binding = bindingIndex++;
	specularLayoutBinding.descriptorCount = 1;
	specularLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	specularLayoutBinding.pImmutableSamplers = nullptr;
	specularLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings.push_back(specularLayoutBinding);

	VkDescriptorSetLayoutBinding materialLayoutBinding = {};
	materialLayoutBinding.binding = bindingIndex++;
	materialLayoutBinding.descriptorCount = 1;
	materialLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	materialLayoutBinding.pImmutableSamplers = nullptr;
	materialLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings.push_back(materialLayoutBinding);

	VkDescriptorSetLayoutBinding depthLayoutBinding = {};
	depthLayoutBinding.binding = bindingIndex++;
	depthLayoutBinding.descriptorCount = 1;
	depthLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	depthLayoutBinding.pImmutableSamplers = nullptr;
	depthLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings.push_back(depthLayoutBinding);

	VkDescriptorSetLayoutBinding cameraLayoutBinding = {};
	cameraLayoutBinding.binding = bindingIndex++;
	cameraLayoutBinding.descriptorCount = 1;
	cameraLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	cameraLayoutBinding.pImmutableSamplers = nullptr;
	cameraLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings.push_back(cameraLayoutBinding);

	VkDescriptorSetLayoutBinding sceneLayoutBinding = {};
	sceneLayoutBinding.binding = bindingIndex++;
	sceneLayoutBinding.descriptorCount = 1;
	sceneLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	sceneLayoutBinding.pImmutableSamplers = nullptr;
	sceneLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings.push_back(sceneLayoutBinding);

	VkDescriptorSetLayoutBinding shadowLayoutBinding = {};
	shadowLayoutBinding.binding = bindingIndex++;
	shadowLayoutBinding.descriptorCount = VkEngine::getEngine().getScene()->getLights().size();
	shadowLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	shadowLayoutBinding.pImmutableSamplers = nullptr;
	shadowLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings.push_back(shadowLayoutBinding);

	VkDescriptorSetLayoutBinding aoLayoutBinding = {};
	aoLayoutBinding.binding = bindingIndex++;
	aoLayoutBinding.descriptorCount = 1;
	aoLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	aoLayoutBinding.pImmutableSamplers = nullptr;
	aoLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings.push_back(aoLayoutBinding);

	descriptorSetLayout = VkEngine::getEngine().getPool()->createDescriptorSetLayout(bindings);
}

void LightingPass::initBufferData()
{
	Camera* camera = VkEngine::getEngine().getScene()->getCamera();
	std::vector<Light*> lights = VkEngine::getEngine().getScene()->getLights();
	sceneUBO.numLights = lights.size();
	
	for (int i = 0; i < sceneUBO.numLights; i++)
	{
		sceneUBO.lights[i].pos = glm::vec4(lights[i]->position, 1);
		sceneUBO.lights[i].ke = glm::vec4(lights[i]->intensity, 1);
		sceneUBO.lights[i].mat = camera->getProjMatrix() * lights[i]->getViewMatrix(camera) * glm::mat4();
	}
	
	sceneUBO.ka = glm::vec4(VkEngine::getEngine().getScene()->getAmbient(), 1);

	updateBuffer(
		VkEngine::getEngine().getDevice(),
		VkEngine::getEngine().getCommandPool(),
		VkEngine::getEngine().getGraphicsQueue(),
		&sceneUBO,
		sizeof(sceneUBO),
		sceneUniformStagingBufferMemory,
		sceneUniformBuffer,
		sceneUniformStagingBuffer);
}

void LightingPass::updateBufferData()
{
	cameraUBO.position = glm::vec4(VkEngine::getEngine().getScene()->getCamera()->frame.origin, 1);

	updateBuffer(
		VkEngine::getEngine().getDevice(),
		VkEngine::getEngine().getCommandPool(),
		VkEngine::getEngine().getGraphicsQueue(),
		&cameraUBO,
		sizeof(cameraUBO),
		cameraUniformStagingBufferMemory,
		cameraUniformBuffer,
		cameraUniformStagingBuffer);
}