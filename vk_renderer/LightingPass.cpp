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
	clearValues[0].color = OPAQUE_BLACK_CLEAR;
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
			&descriptorSet,
			0,
			nullptr);

		vkCmdDrawIndexed(commandBuffers[i], quad->indices.size(), 1, 0, 0, 1);

		vkCmdEndRenderPass(commandBuffers[i]);

		VK_CHECK(vkEndCommandBuffer(commandBuffers[i]));
	}
}

void LightingPass::initDescriptorSet()
{
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = VkEngine::getEngine().getDescriptorPool();
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout;

	VK_CHECK(vkAllocateDescriptorSets(VkEngine::getEngine().getDevice(), &allocInfo, &descriptorSet));

	std::array<VkWriteDescriptorSet, 6> descriptorWrites = {};

	VkDescriptorImageInfo colorImageInfo = {};
	colorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	colorImageInfo.imageView = prevPassGBuffer->colorAttachment.imageView;
	colorImageInfo.sampler = prevPassGBuffer->colorAttachment.imageSampler;

	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = descriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pImageInfo = &colorImageInfo;

	VkDescriptorImageInfo positionImageInfo = {};
	positionImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	positionImageInfo.imageView = prevPassGBuffer->positionAttachment.imageView;
	positionImageInfo.sampler = prevPassGBuffer->positionAttachment.imageSampler;

	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].dstSet = descriptorSet;
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pImageInfo = &positionImageInfo;

	VkDescriptorImageInfo normalImageInfo = {};
	normalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	normalImageInfo.imageView = prevPassGBuffer->normalAttachment.imageView;
	normalImageInfo.sampler = prevPassGBuffer->normalAttachment.imageSampler;

	descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[2].dstSet = descriptorSet;
	descriptorWrites[2].dstBinding = 2;
	descriptorWrites[2].dstArrayElement = 0;
	descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[2].descriptorCount = 1;
	descriptorWrites[2].pImageInfo = &normalImageInfo;

	VkDescriptorImageInfo depthImageInfo = {};
	depthImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	depthImageInfo.imageView = prevPassGBuffer->depthAttachment.imageView;
	depthImageInfo.sampler = prevPassGBuffer->depthAttachment.imageSampler;

	descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[3].dstSet = descriptorSet;
	descriptorWrites[3].dstBinding = 3;
	descriptorWrites[3].dstArrayElement = 0;
	descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[3].descriptorCount = 1;
	descriptorWrites[3].pImageInfo = &depthImageInfo;

	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = lightsUniformBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(LPLightsUniformBufferObject);

	descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[4].dstSet = descriptorSet;
	descriptorWrites[4].dstBinding = 4;
	descriptorWrites[4].dstArrayElement = 0;
	descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[4].descriptorCount = 1;
	descriptorWrites[4].pBufferInfo = &bufferInfo;

	bufferInfo = {};
	bufferInfo.buffer = cameraUniformBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(LPCameraUniformBufferObject);

	descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[5].dstSet = descriptorSet;
	descriptorWrites[5].dstBinding = 5;
	descriptorWrites[5].dstArrayElement = 0;
	descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[5].descriptorCount = 1;
	descriptorWrites[5].pBufferInfo = &bufferInfo;

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
}

void LightingPass::initDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings(5);

	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = nullptr;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings[0] = uboLayoutBinding;

	VkDescriptorSetLayoutBinding colorSamplerLayoutBinding = {};
	colorSamplerLayoutBinding.binding = 1;
	colorSamplerLayoutBinding.descriptorCount = 1;
	colorSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	colorSamplerLayoutBinding.pImmutableSamplers = nullptr;
	colorSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings[1] = colorSamplerLayoutBinding;

	VkDescriptorSetLayoutBinding positionSamplerLayoutBinding = {};
	positionSamplerLayoutBinding.binding = 2;
	positionSamplerLayoutBinding.descriptorCount = 1;
	positionSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	positionSamplerLayoutBinding.pImmutableSamplers = nullptr;
	positionSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings[2] = positionSamplerLayoutBinding;

	VkDescriptorSetLayoutBinding normalSamplerLayoutBinding = {};
	normalSamplerLayoutBinding.binding = 3;
	normalSamplerLayoutBinding.descriptorCount = 1;
	normalSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	normalSamplerLayoutBinding.pImmutableSamplers = nullptr;
	normalSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings[3] = normalSamplerLayoutBinding;

	VkDescriptorSetLayoutBinding depthLayoutBinding = {};
	depthLayoutBinding.binding = 4;
	depthLayoutBinding.descriptorCount = 1;
	depthLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	depthLayoutBinding.pImmutableSamplers = nullptr;
	depthLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings[4] = depthLayoutBinding;

	descriptorSetLayout = VkEngine::getEngine().getPool()->createDescriptorSetLayout(bindings);
}

void LightingPass::initBufferData()
{
	std::vector<Light*> lights = VkEngine::getEngine().getScene()->getLights();

	lightsUBO.count = lights.size();

	for (int i = 0; i < lightsUBO.count; i++)
	{
		lightsUBO.positions[i] = lights[i]->position;
		lightsUBO.intensities[i] = lights[i]->intensity;
	}

	uint8_t* data;
	VK_CHECK(vkMapMemory(VkEngine::getEngine().getDevice(), lightsUniformStagingBufferMemory, 0, sizeof(lightsUBO), 0, (void**)&data));
	memcpy(data, &lightsUBO, sizeof(lightsUBO));
	vkUnmapMemory(VkEngine::getEngine().getDevice(), lightsUniformStagingBufferMemory);

	copyBuffer(
		VkEngine::getEngine().getDevice(),
		VkEngine::getEngine().getCommandPool(),
		VkEngine::getEngine().getGraphicsQueue(),
		lightsUniformStagingBuffer,
		lightsUniformBuffer,
		sizeof(lightsUBO));
}

void LightingPass::updateBufferData()
{
	cameraUBO.position = VkEngine::getEngine().getScene()->getCamera()->frame.origin;

	uint8_t* data;
	VK_CHECK(vkMapMemory(VkEngine::getEngine().getDevice(), cameraUniformStagingBufferMemory, 0, sizeof(cameraUBO), 0, (void**)&data));
	memcpy(data, &cameraUBO, sizeof(cameraUBO));
	vkUnmapMemory(VkEngine::getEngine().getDevice(), cameraUniformStagingBufferMemory);

	copyBuffer(
		VkEngine::getEngine().getDevice(),
		VkEngine::getEngine().getCommandPool(),
		VkEngine::getEngine().getGraphicsQueue(),
		cameraUniformStagingBuffer,
		cameraUniformBuffer,
		sizeof(cameraUBO));
}