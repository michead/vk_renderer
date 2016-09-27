#include "GeometryPass.h"

#include "Camera.h"
#include "Scene.h"
#include "VkPool.h"


void GeometryPass::initAttachments()
{
	gBuffer.init();
}

void GeometryPass::initCommandBuffers()
{
	static bool firstTime = true;
	if (!firstTime)
	{
		vkFreeCommandBuffers(
			VkEngine::getEngine().getDevice(),
			VkEngine::getEngine().getCommandPool(),
			1,
			&commandBuffer);
	}
	else
	{
		firstTime = false;
	}

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = VkEngine::getEngine().getCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VK_CHECK(vkAllocateCommandBuffers(VkEngine::getEngine().getDevice(), &allocInfo, &commandBuffer));

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkExtent2D extent = VkEngine::getEngine().getSwapchainExtent();

	VkRect2D renderArea = {};
	renderArea.extent = extent;
	renderArea.offset = { 0, 0 };

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = gBuffer.renderPass;
	renderPassInfo.framebuffer = gBuffer.framebuffer;
	renderPassInfo.renderArea = renderArea;

	std::array<VkClearValue, 4> clearValues = {};
	clearValues[0].color = OPAQUE_BLACK_CLEAR;
	clearValues[1].color = OPAQUE_BLACK_CLEAR;
	clearValues[2].color = OPAQUE_BLACK_CLEAR;
	clearValues[3].depthStencil = DEPTH_STENCIL_CLEAR;

	renderPassInfo.clearValueCount = clearValues.size();
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport = {};
	viewport.width = extent.width;
	viewport.height = extent.height;
	viewport.minDepth = 0;
	viewport.maxDepth = 1;

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &renderArea);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	for (const auto& elem : VkEngine::getEngine().getScene()->getElems())
	{
		VkBuffer vertexBuffers[] = { elem->getVertexBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, elem->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			1,
			&descriptorSet,
			0,
			nullptr);

		vkCmdDrawIndexed(commandBuffer, elem->getMesh().indices.size(), 1, 0, 0, 0);
	}

	vkCmdEndRenderPass(commandBuffer);

	VK_CHECK(vkEndCommandBuffer(commandBuffer));
}

void GeometryPass::initDescriptorSet()
{
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = VkEngine::getEngine().getDescriptorPool();
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout;

	VK_CHECK(vkAllocateDescriptorSets(VkEngine::getEngine().getDevice(), &allocInfo, &descriptorSet));

	std::map<std::string, Texture*> textureMap = VkEngine::getEngine().getScene()->getTextureMap();
	std::vector<VkDescriptorImageInfo> imageInfos(textureMap.size());

	uint16_t i = 0;
	std::map<std::string, Texture*>::iterator it;
	for (it = textureMap.begin(); it != textureMap.end(); it++)
	{
		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = it->second->getImageView();
		imageInfo.sampler = it->second->getSampler();

		imageInfos[i] = imageInfo;

		i++;
	}

	std::vector<VkWriteDescriptorSet> descriptorWrites(textureMap.size() + 2);

	i = 0;
	for (it = textureMap.begin(); it != textureMap.end(); it++)
	{
		descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[i].dstSet = descriptorSet;
		descriptorWrites[i].dstBinding = i;
		descriptorWrites[i].dstArrayElement = 0;
		descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[i].descriptorCount = 1;
		descriptorWrites[i].pImageInfo = &imageInfos[i - 1];

		i++;
	}

	VkDescriptorBufferInfo cameraBufferInfo = {};
	cameraBufferInfo.buffer = cameraUniformBuffer;
	cameraBufferInfo.offset = 0;
	cameraBufferInfo.range = sizeof(GPCameraUniformBufferObject);

	descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[i].dstSet = descriptorSet;
	descriptorWrites[i].dstBinding = 0;
	descriptorWrites[i].dstArrayElement = 0;
	descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[i].descriptorCount = 1;
	descriptorWrites[i].pBufferInfo = &cameraBufferInfo;

	i++;

	VkDescriptorBufferInfo materialBufferInfo = {};
	materialBufferInfo.buffer = cameraUniformBuffer;
	materialBufferInfo.offset = 0;
	materialBufferInfo.range = sizeof(GPMaterialUniformBufferObject);

	descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[i].dstSet = descriptorSet;
	descriptorWrites[i].dstBinding = 0;
	descriptorWrites[i].dstArrayElement = 0;
	descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[i].descriptorCount = 1;
	descriptorWrites[i].pBufferInfo = &materialBufferInfo;

	vkUpdateDescriptorSets(VkEngine::getEngine().getDevice(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

void GeometryPass::updateBufferData()
{
	GPCameraUniformBufferObject ubo = {};
	ubo.model = glm::mat4(); // Useless
	ubo.view = VkEngine::getEngine().getScene()->getCamera()->getViewMatrix();
	ubo.proj = VkEngine::getEngine().getScene()->getCamera()->getProjMatrix();

	void* data;
	VK_CHECK(vkMapMemory(VkEngine::getEngine().getDevice(), cameraUniformStagingBufferMemory, 0, sizeof(ubo), 0, &data));
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(VkEngine::getEngine().getDevice(), cameraUniformStagingBufferMemory);

	copyBuffer(
		VkEngine::getEngine().getDevice(),
		VkEngine::getEngine().getCommandPool(),
		VkEngine::getEngine().getGraphicsQueue(),
		cameraUniformStagingBuffer,
		cameraUniformBuffer,
		sizeof(ubo));
}

void GeometryPass::initGraphicsPipeline()
{
	std::vector<char> vs = readFile(vsPath);
	std::vector<char> fs = readFile(fsPath);
	std::vector<char> gs;

	if (!gsPath.empty())
	{
		gs = readFile(gsPath);
	}

	PipelineData pipelineData = VkEngine::getEngine().getPool()->createPipeline(
		gBuffer.renderPass,
		descriptorSetLayout,
		VkEngine::getEngine().getSwapchainExtent(),
		vs,
		fs,
		gs);

	pipeline = pipelineData.pipeline;
	pipelineLayout = pipelineData.pipelineLayout;
}

void GeometryPass::initUniformBuffer()
{
	VkDeviceSize bufferSize = sizeof(GPCameraUniformBufferObject);

	std::vector<BufferData> bufferDataVec = VkEngine::getEngine().getPool()->createUniformBuffer(bufferSize, true);
	cameraUniformStagingBuffer = bufferDataVec[0].buffer;
	cameraUniformStagingBufferMemory = bufferDataVec[0].bufferMemory;
	cameraUniformBuffer = bufferDataVec[1].buffer;
	cameraUniformBufferMemory = bufferDataVec[1].bufferMemory;

	bufferSize = sizeof(GPMaterialUniformBufferObject);

	bufferDataVec = VkEngine::getEngine().getPool()->createUniformBuffer(bufferSize, true);
	materialUniformStagingBuffer = bufferDataVec[0].buffer;
	materialUniformStagingBufferMemory = bufferDataVec[0].bufferMemory;
	materialUniformBuffer = bufferDataVec[1].buffer;
	materialUniformBufferMemory = bufferDataVec[1].bufferMemory;
}

void GeometryPass::initDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings(4);

	VkDescriptorSetLayoutBinding samplerAlbedoLayoutBinding = {};
	samplerAlbedoLayoutBinding.binding = 0;
	samplerAlbedoLayoutBinding.descriptorCount = 1;
	samplerAlbedoLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerAlbedoLayoutBinding.pImmutableSamplers = nullptr;
	samplerAlbedoLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings[0] = samplerAlbedoLayoutBinding;

	VkDescriptorSetLayoutBinding samplerNormalLayoutBinding = {};
	samplerNormalLayoutBinding.binding = 1;
	samplerNormalLayoutBinding.descriptorCount = 1;
	samplerNormalLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerNormalLayoutBinding.pImmutableSamplers = nullptr;
	samplerNormalLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings[1] = samplerNormalLayoutBinding;

	VkDescriptorSetLayoutBinding cameraUBOLayoutBinding = {};
	cameraUBOLayoutBinding.binding = 2;
	cameraUBOLayoutBinding.descriptorCount = 1;
	cameraUBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	cameraUBOLayoutBinding.pImmutableSamplers = nullptr;
	cameraUBOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	bindings[2] = cameraUBOLayoutBinding;

	VkDescriptorSetLayoutBinding materialUBOLayoutBinding = {};
	materialUBOLayoutBinding.binding = 3;
	materialUBOLayoutBinding.descriptorCount = 1;
	materialUBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	materialUBOLayoutBinding.pImmutableSamplers = nullptr;
	materialUBOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	bindings[3] = materialUBOLayoutBinding;

	descriptorSetLayout = VkEngine::getEngine().getPool()->createDescriptorSetLayout(bindings);
}