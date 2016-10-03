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

	std::vector<VkWriteDescriptorSet> descriptorWrites(11);

	VkDescriptorImageInfo colorImageInfo = {};
	colorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	colorImageInfo.imageView = prevPassGBuffer->attachments[GBUFFER_COLOR_ATTACH_ID].imageView;
	colorImageInfo.sampler = prevPassGBuffer->attachments[GBUFFER_COLOR_ATTACH_ID].imageSampler;

	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = descriptorSets[0];
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pImageInfo = &colorImageInfo;

	VkDescriptorImageInfo positionImageInfo = {};
	positionImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	positionImageInfo.imageView = prevPassGBuffer->attachments[GBUFFER_POSITION_ATTACH_ID].imageView;
	positionImageInfo.sampler = prevPassGBuffer->attachments[GBUFFER_POSITION_ATTACH_ID].imageSampler;

	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].dstSet = descriptorSets[0];
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pImageInfo = &positionImageInfo;

	VkDescriptorImageInfo normalImageInfo = {};
	normalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	normalImageInfo.imageView = prevPassGBuffer->attachments[GBUFFER_NORMAL_ATTACH_ID].imageView;
	normalImageInfo.sampler = prevPassGBuffer->attachments[GBUFFER_NORMAL_ATTACH_ID].imageSampler;

	descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[2].dstSet = descriptorSets[0];
	descriptorWrites[2].dstBinding = 2;
	descriptorWrites[2].dstArrayElement = 0;
	descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[2].descriptorCount = 1;
	descriptorWrites[2].pImageInfo = &normalImageInfo;

	VkDescriptorImageInfo tangentImageInfo = {};
	tangentImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	tangentImageInfo.imageView = prevPassGBuffer->attachments[GBUFFER_TANGENT_ATTACH_ID].imageView;
	tangentImageInfo.sampler = prevPassGBuffer->attachments[GBUFFER_TANGENT_ATTACH_ID].imageSampler;

	descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[3].dstSet = descriptorSets[0];
	descriptorWrites[3].dstBinding = 3;
	descriptorWrites[3].dstArrayElement = 0;
	descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[3].descriptorCount = 1;
	descriptorWrites[3].pImageInfo = &tangentImageInfo;

	VkDescriptorImageInfo specularImageInfo = {};
	specularImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	specularImageInfo.imageView = prevPassGBuffer->attachments[GBUFFER_SPECULAR_ATTACH_ID].imageView;
	specularImageInfo.sampler = prevPassGBuffer->attachments[GBUFFER_SPECULAR_ATTACH_ID].imageSampler;

	descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[4].dstSet = descriptorSets[0];
	descriptorWrites[4].dstBinding = 4;
	descriptorWrites[4].dstArrayElement = 0;
	descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[4].descriptorCount = 1;
	descriptorWrites[4].pImageInfo = &specularImageInfo;

	VkDescriptorImageInfo materialImageInfo = {};
	materialImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	materialImageInfo.imageView = prevPassGBuffer->attachments[GBUFFER_MATERIAL_ATTACH_ID].imageView;
	materialImageInfo.sampler = prevPassGBuffer->attachments[GBUFFER_MATERIAL_ATTACH_ID].imageSampler;

	descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[5].dstSet = descriptorSets[0];
	descriptorWrites[5].dstBinding = 5;
	descriptorWrites[5].dstArrayElement = 0;
	descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[5].descriptorCount = 1;
	descriptorWrites[5].pImageInfo = &materialImageInfo;

	VkDescriptorImageInfo depthImageInfo = {};
	depthImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	depthImageInfo.imageView = prevPassGBuffer->attachments[GBUFFER_DEPTH_ATTACH_ID].imageView;
	depthImageInfo.sampler = prevPassGBuffer->attachments[GBUFFER_DEPTH_ATTACH_ID].imageSampler;

	descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[6].dstSet = descriptorSets[0];
	descriptorWrites[6].dstBinding = 6;
	descriptorWrites[6].dstArrayElement = 0;
	descriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[6].descriptorCount = 1;
	descriptorWrites[6].pImageInfo = &depthImageInfo;

	VkDescriptorBufferInfo cameraBufferInfo = {};
	cameraBufferInfo.buffer = cameraUniformBuffer;
	cameraBufferInfo.offset = 0;
	cameraBufferInfo.range = sizeof(LPCameraUniformBufferObject);

	descriptorWrites[7].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[7].dstSet = descriptorSets[0];
	descriptorWrites[7].dstBinding = 7;
	descriptorWrites[7].dstArrayElement = 0;
	descriptorWrites[7].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[7].descriptorCount = 1;
	descriptorWrites[7].pBufferInfo = &cameraBufferInfo;

	VkDescriptorBufferInfo lightsBufferInfo = {};
	lightsBufferInfo.buffer = lightsUniformBuffer;
	lightsBufferInfo.offset = 0;
	lightsBufferInfo.range = sizeof(LPLightsUniformBufferObject);

	descriptorWrites[8].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[8].dstSet = descriptorSets[0];
	descriptorWrites[8].dstBinding = 8;
	descriptorWrites[8].dstArrayElement = 0;
	descriptorWrites[8].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[8].descriptorCount = 1;
	descriptorWrites[8].pBufferInfo = &lightsBufferInfo;

	VkDescriptorBufferInfo sceneBufferInfo = {};
	sceneBufferInfo.buffer = sceneUniformBuffer;
	sceneBufferInfo.offset = 0;
	sceneBufferInfo.range = sizeof(LPSceneUniformBufferObject);

	descriptorWrites[9].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[9].dstSet = descriptorSets[0];
	descriptorWrites[9].dstBinding = 9;
	descriptorWrites[9].dstArrayElement = 0;
	descriptorWrites[9].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[9].descriptorCount = 1;
	descriptorWrites[9].pBufferInfo = &sceneBufferInfo;

	std::vector<VkDescriptorImageInfo> shadowImageInfos;
	for (size_t l = 0; l < numShadowMaps; l++)
	{
		VkDescriptorImageInfo shadowImageInfo = {};
		shadowImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		shadowImageInfo.imageView = shadowMaps[l].imageView;
		shadowImageInfo.sampler = shadowMaps[l].imageSampler;

		shadowImageInfos.push_back(shadowImageInfo);
	}

	descriptorWrites[10].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[10].dstSet = descriptorSets[0];
	descriptorWrites[10].dstBinding = 10;
	descriptorWrites[10].dstArrayElement = 0;
	descriptorWrites[10].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[10].descriptorCount = shadowImageInfos.size();
	descriptorWrites[10].pImageInfo = shadowImageInfos.data();

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
	VkDeviceSize bufferSize = sizeof(LPLightsUniformBufferObject);
	
	std::vector<BufferData> bufferDataVec = VkEngine::getEngine().getPool()->createUniformBuffer(bufferSize, true);
	lightsUniformStagingBuffer = bufferDataVec[0].buffer;
	lightsUniformStagingBufferMemory = bufferDataVec[0].bufferMemory;
	lightsUniformBuffer = bufferDataVec[1].buffer;
	lightsUniformBufferMemory = bufferDataVec[1].bufferMemory;

	bufferSize = sizeof(LPCameraUniformBufferObject);

	bufferDataVec = VkEngine::getEngine().getPool()->createUniformBuffer(bufferSize, true);
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
	std::vector<VkDescriptorSetLayoutBinding> bindings(11);

	VkDescriptorSetLayoutBinding colorSamplerLayoutBinding = {};
	colorSamplerLayoutBinding.binding = 0;
	colorSamplerLayoutBinding.descriptorCount = 1;
	colorSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	colorSamplerLayoutBinding.pImmutableSamplers = nullptr;
	colorSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings[0] = colorSamplerLayoutBinding;

	VkDescriptorSetLayoutBinding positionSamplerLayoutBinding = {};
	positionSamplerLayoutBinding.binding = 1;
	positionSamplerLayoutBinding.descriptorCount = 1;
	positionSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	positionSamplerLayoutBinding.pImmutableSamplers = nullptr;
	positionSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings[1] = positionSamplerLayoutBinding;

	VkDescriptorSetLayoutBinding normalSamplerLayoutBinding = {};
	normalSamplerLayoutBinding.binding = 2;
	normalSamplerLayoutBinding.descriptorCount = 1;
	normalSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	normalSamplerLayoutBinding.pImmutableSamplers = nullptr;
	normalSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings[2] = normalSamplerLayoutBinding;

	VkDescriptorSetLayoutBinding tangentLayoutBinding = {};
	tangentLayoutBinding.binding = 3;
	tangentLayoutBinding.descriptorCount = 1;
	tangentLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	tangentLayoutBinding.pImmutableSamplers = nullptr;
	tangentLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings[3] = tangentLayoutBinding;

	VkDescriptorSetLayoutBinding specularLayoutBinding = {};
	specularLayoutBinding.binding = 4;
	specularLayoutBinding.descriptorCount = 1;
	specularLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	specularLayoutBinding.pImmutableSamplers = nullptr;
	specularLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings[4] = specularLayoutBinding;

	VkDescriptorSetLayoutBinding materialLayoutBinding = {};
	materialLayoutBinding.binding = 5;
	materialLayoutBinding.descriptorCount = 1;
	materialLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	materialLayoutBinding.pImmutableSamplers = nullptr;
	materialLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings[5] = materialLayoutBinding;

	VkDescriptorSetLayoutBinding depthLayoutBinding = {};
	depthLayoutBinding.binding = 6;
	depthLayoutBinding.descriptorCount = 1;
	depthLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	depthLayoutBinding.pImmutableSamplers = nullptr;
	depthLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings[6] = depthLayoutBinding;

	VkDescriptorSetLayoutBinding cameraLayoutBinding = {};
	cameraLayoutBinding.binding = 7;
	cameraLayoutBinding.descriptorCount = 1;
	cameraLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	cameraLayoutBinding.pImmutableSamplers = nullptr;
	cameraLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings[7] = cameraLayoutBinding;

	VkDescriptorSetLayoutBinding lightsLayoutBinding = {};
	lightsLayoutBinding.binding = 8;
	lightsLayoutBinding.descriptorCount = 1;
	lightsLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	lightsLayoutBinding.pImmutableSamplers = nullptr;
	lightsLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings[8] = lightsLayoutBinding;

	VkDescriptorSetLayoutBinding sceneLayoutBinding = {};
	sceneLayoutBinding.binding = 9;
	sceneLayoutBinding.descriptorCount = 1;
	sceneLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	sceneLayoutBinding.pImmutableSamplers = nullptr;
	sceneLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings[9] = sceneLayoutBinding;

	VkDescriptorSetLayoutBinding shadowLayoutBinding = {};
	shadowLayoutBinding.binding = 10;
	shadowLayoutBinding.descriptorCount = VkEngine::getEngine().getScene()->getLights().size();
	shadowLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	shadowLayoutBinding.pImmutableSamplers = nullptr;
	shadowLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings[10] = shadowLayoutBinding;

	descriptorSetLayout = VkEngine::getEngine().getPool()->createDescriptorSetLayout(bindings);
}

void LightingPass::initBufferData()
{
	std::vector<Light*> lights = VkEngine::getEngine().getScene()->getLights();
	lightsUBO.count = lights.size();

	for (int i = 0; i < lightsUBO.count; i++)
	{
		lightsUBO.positions[i] = glm::vec4(lights[i]->position, 1);
		lightsUBO.intensities[i] = glm::vec4(lights[i]->intensity, 1);
	}
	
	updateBuffer(
		VkEngine::getEngine().getDevice(),
		VkEngine::getEngine().getCommandPool(),
		VkEngine::getEngine().getGraphicsQueue(),
		&lightsUBO,
		sizeof(lightsUBO),
		lightsUniformStagingBufferMemory,
		lightsUniformBuffer,
		lightsUniformStagingBuffer);

	sceneUBO.ambient = glm::vec4(VkEngine::getEngine().getScene()->getAmbient(), 1);

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