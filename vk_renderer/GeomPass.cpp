#include "GeomPass.h"

#include "Camera.h"
#include "Scene.h"
#include "VkPool.h"


void GeomPass::initAttachments()
{
	VkAttachmentDescription colorAttachment = {};
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

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = findDepthFormat(VkEngine::getEngine().getPhysicalDevice());
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subPass = {};
	subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subPass.colorAttachmentCount = 1;
	subPass.pColorAttachments = &colorAttachmentRef;
	subPass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = attachments.size();
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subPass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	renderPass = VkEngine::getEngine().getPool()->createRenderPass(renderPassInfo);
}

void GeomPass::initCommandBuffers()
{
	if (commandBuffers.size() > 0)
	{
		vkFreeCommandBuffers(
			VkEngine::getEngine().getDevice(),
			VkEngine::getEngine().getCommandPool(),
			commandBuffers.size(),
			commandBuffers.data());
	}
	else
	{
		commandBuffers.resize(1);
	}

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = VkEngine::getEngine().getCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t) 1;

	VK_CHECK(vkAllocateCommandBuffers(VkEngine::getEngine().getDevice(), &allocInfo, commandBuffers.data()));

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	vkBeginCommandBuffer(commandBuffers.front(), &beginInfo);

	VkExtent2D extent = VkEngine::getEngine().getSwapchainExtent();

	VkRect2D renderArea = {};
	renderArea.extent = extent;
	renderArea.offset = { 0, 0 };

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = framebuffers.front();
	renderPassInfo.renderArea = renderArea;

	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = OPAQUE_BLACK_CLEAR;
	clearValues[1].depthStencil = DEPTH_STENCIL_CLEAR;

	renderPassInfo.clearValueCount = clearValues.size();
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffers[0], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport = {};
	viewport.width = extent.width;
	viewport.height = extent.height;
	viewport.minDepth = 0;
	viewport.maxDepth = 1;

	vkCmdSetViewport(commandBuffers.front(), 0, 1, &viewport);
	vkCmdSetScissor(commandBuffers.front(), 0, 1, &renderArea);
	vkCmdBindPipeline(commandBuffers.front(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	for (const auto& elem : VkEngine::getEngine().getScene()->getElems())
	{
		VkBuffer vertexBuffers[] = { elem->getVertexBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffers.front(), 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffers.front(), elem->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(
			commandBuffers.front(),
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			1,
			&descriptorSet,
			0,
			nullptr);

		vkCmdDrawIndexed(commandBuffers.front(), elem->getMesh().indices.size(), 1, 0, 0, 0);
	}

	vkCmdEndRenderPass(commandBuffers.front());

	VK_CHECK(vkEndCommandBuffer(commandBuffers.front()));
}

void GeomPass::initFramebuffers()
{
	auto gBuffers = VkEngine::getEngine().getGBuffers();
	size_t numGBuffers = gBuffers.size();

	framebuffers.resize(numGBuffers);

	for (size_t i = 0; i < numGBuffers; i++)
	{
		framebuffers[i] = gBuffers[i].framebuffer;
	}
}

void GeomPass::initDescriptorSet()
{
	VkDescriptorSetLayout descriptorSetLayout = VkEngine::getEngine().getOneStageDescriptorSetLayout();

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = VkEngine::getEngine().getDescriptorPool();
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout;

	VK_CHECK(vkAllocateDescriptorSets(VkEngine::getEngine().getDevice(), &allocInfo, &descriptorSet));

	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = uniformBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(UniformBufferObject);

	uint16_t i = 0;
	std::map<std::string, Texture*>::iterator it;
	std::map<std::string, Texture*> textureMap = VkEngine::getEngine().getScene()->getTextureMap();
	std::vector<VkDescriptorImageInfo> imageInfos(textureMap.size());
	for (it = textureMap.begin(); it != textureMap.end(); it++)
	{
		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = it->second->getImageView();
		imageInfo.sampler = it->second->getSampler();

		imageInfos[i] = imageInfo;

		i++;
	}

	i = 0;
	std::vector<VkWriteDescriptorSet> descriptorWrites(2 * textureMap.size());
	for (it = textureMap.begin(); it != textureMap.end(); it++)
	{
		descriptorWrites[2 * i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2 * i].dstSet = descriptorSet;
		descriptorWrites[2 * i].dstBinding = 0;
		descriptorWrites[2 * i].dstArrayElement = 0;
		descriptorWrites[2 * i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[2 * i].descriptorCount = 1;
		descriptorWrites[2 * i].pBufferInfo = &bufferInfo;

		descriptorWrites[2 * i + 1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2 * i + 1].dstSet = descriptorSet;
		descriptorWrites[2 * i + 1].dstBinding = 1;
		descriptorWrites[2 * i + 1].dstArrayElement = 0;
		descriptorWrites[2 * i + 1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[2 * i + 1].descriptorCount = 1;
		descriptorWrites[2 * i + 1].pImageInfo = &imageInfos[i];

		i++;
	}

	vkUpdateDescriptorSets(VkEngine::getEngine().getDevice(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

void GeomPass::updateData()
{
	UniformBufferObject ubo = {};
	ubo.model = glm::mat4(); // Useless
	ubo.view = VkEngine::getEngine().getScene()->getCamera()->getViewMatrix();
	ubo.proj = VkEngine::getEngine().getScene()->getCamera()->getProjMatrix();

	void* data;
	VK_CHECK(vkMapMemory(VkEngine::getEngine().getDevice(), uniformStagingBufferMemory, 0, sizeof(ubo), 0, &data));
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(VkEngine::getEngine().getDevice(), uniformStagingBufferMemory);

	copyBuffer(
		VkEngine::getEngine().getDevice(),
		VkEngine::getEngine().getCommandPool(),
		VkEngine::getEngine().getGraphicsQueue(),
		uniformStagingBuffer,
		uniformBuffer,
		sizeof(ubo));
}

void GeomPass::initGraphicsPipeline()
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
		VkEngine::getEngine().getOneStageDescriptorSetLayout(),
		VkEngine::getEngine().getSwapchainExtent(),
		vs,
		fs,
		gs);

	pipeline = pipelineData.pipeline;
	pipelineLayout = pipelineData.pipelineLayout;
}