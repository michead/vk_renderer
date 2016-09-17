#include "RenderPass.h"

#include <map>

#include "Camera.h"
#include "GeomStructs.h"
#include "Scene.h"
#include "SceneElem.h"
#include "Texture.h"
#include "VkUtils.h"
#include "VkPool.h"


void RenderPass::init()
{
	initAttachments();
	initDescriptorSetLayout();
	initGraphicsPipeline();
	initDepthResources();
	initFramebuffers();
	initTextures();
	initMeshBuffers();
	initUniformBuffer();
	initDescriptorSet();
	initCommandBuffers();
}

void RenderPass::initAttachments()
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

void RenderPass::initGraphicsPipeline()
{
	std::vector<char> vs = readFile(vsPath);
	std::vector<char> fs = readFile(fsPath);
	std::vector<char> gs;

	if (!gsPath.empty())
	{
		gs = readFile(gsPath);
	}

	PipelineData pipelineData = VkEngine().getEngine().getPool()->createPipeline(
		renderPass, 
		descriptorSetLayout, 
		VkEngine::getEngine().getSwapchainExtent(),
		vs, 
		fs, 
		gs);

	graphicsPipeline = pipelineData.pipeline;
	pipelineLayout = pipelineData.pipelineLayout;
}

void RenderPass::initDepthResources()
{
	DepthData depthData = VkEngine::getEngine().getPool()->createDepthResources();
	depthImage = depthData.image;
	depthImageView = depthData.imageView;
	depthImageMemory = depthData.imageMemory;
}

void RenderPass::initMeshBuffers()
{
	std::vector<SceneElem*> elems = VkEngine::getEngine().getScene()->getElems();
	for (const auto& elem : elems)
	{
		elem->initBuffers();
	}
}

void RenderPass::initTextures()
{
	std::map<std::string, Texture*> textureMap = VkEngine::getEngine().getScene()->getTextureMap();
	for (auto textureEntry : textureMap)
	{
		textureEntry.second->init();
	}
}

void RenderPass::initFramebuffers()
{
	auto imageViews = VkEngine::getEngine().getSwapchainImageViews();
	size_t numImageViews = imageViews.size();

	swapchainFramebuffers.resize(numImageViews);

	for (size_t i = 0; i < numImageViews; i++)
	{
		std::array<VkImageView, 2> attachments = { imageViews[i], depthImageView };

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = attachments.size();
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = VkEngine::getEngine().getSwapchainExtent().width;
		framebufferInfo.height = VkEngine::getEngine().getSwapchainExtent().height;
		framebufferInfo.layers = 1;

		VK_CHECK(vkCreateFramebuffer(VkEngine::getEngine().getDevice(), &framebufferInfo, nullptr, &swapchainFramebuffers[i]));
	}
}

void RenderPass::initCommandBuffers()
{
	if (commandBuffers.size() > 0)
	{
		vkFreeCommandBuffers(
			VkEngine::getEngine().getDevice(), 
			VkEngine::getEngine().getCommandPool(), 
			commandBuffers.size(), 
			commandBuffers.data());
	}

	commandBuffers.resize(swapchainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = VkEngine::getEngine().getCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

	VK_CHECK(vkAllocateCommandBuffers(VkEngine::getEngine().getDevice(), &allocInfo, commandBuffers.data()));

	for (size_t i = 0; i < commandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapchainFramebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = VkEngine::getEngine().getSwapchainExtent();

		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { 0.f, 0.f, 0.f, 1.f };
		clearValues[1].depthStencil = { 1.f, 0 };

		renderPassInfo.clearValueCount = clearValues.size();
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		for (const auto& elem : VkEngine::getEngine().getScene()->getElems())
		{
			VkBuffer vertexBuffers[] = { elem->getVertexBuffer() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commandBuffers[i], elem->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdBindDescriptorSets(
				commandBuffers[i], 
				VK_PIPELINE_BIND_POINT_GRAPHICS, 
				pipelineLayout, 
				0, 
				1, 
				&descriptorSet, 
				0, 
				nullptr);

			vkCmdDrawIndexed(commandBuffers[i], elem->getMesh().indices.size(), 1, 0, 0, 0);
		}

		vkCmdEndRenderPass(commandBuffers[i]);

		VK_CHECK(vkEndCommandBuffer(commandBuffers[i]));
	}
}

VkResult RenderPass::run()
{
	uint32_t imageIndex = VkEngine::getEngine().getSwapchainImageIndex();

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { VkEngine::getEngine().getImageAvailableSemaphore() };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = { VkEngine::getEngine().getRenderFinishedSemaphore() };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VK_CHECK(vkQueueSubmit(VkEngine::getEngine().getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { VkEngine::getEngine().getSwapchain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	presentInfo.pResults = nullptr;

	return vkQueuePresentKHR(VkEngine::getEngine().getPresentationQueue(), &presentInfo);
}

void RenderPass::updateData()
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

void RenderPass::initDescriptorSet()
{
	std::map<std::string, Texture*>::iterator it;
	std::map<std::string, Texture*> textureMap = VkEngine::getEngine().getScene()->getTextureMap();

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

void RenderPass::initDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = nullptr;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = bindings.size();
	layoutInfo.pBindings = bindings.data();

	VK_CHECK(vkCreateDescriptorSetLayout(VkEngine::getEngine().getDevice(), &layoutInfo, nullptr, &descriptorSetLayout));

}

void RenderPass::initUniformBuffer()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	std::array<BufferData, 2> bufferDataArray = VkEngine::getEngine().getPool()->createUniformBuffer(bufferSize);
	uniformStagingBuffer = bufferDataArray[0].buffer;
	uniformStagingBufferMemory = bufferDataArray[0].bufferMemory;
	uniformBuffer = bufferDataArray[1].buffer;
	uniformBufferMemory = bufferDataArray[1].bufferMemory;
}