#include "RenderPass.h"

#include <map>

#include "Camera.h"
#include "GeomStructs.h"
#include "Scene.h"
#include "SceneElem.h"
#include "Texture.h"
#include "VkUtils.h"


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

	VK_CHECK(vkCreateRenderPass(VkEngine::getEngine().getDevice(), &renderPassInfo, nullptr, &renderPass));
}

void RenderPass::initGraphicsPipeline()
{
	std::vector<char> vsCode = readFile(vsPath);
	std::vector<char> fsCode = readFile(fsPath);

	VkWrap<VkShaderModule> vsModule { VkEngine::getEngine().getDevice(), vkDestroyShaderModule };
	VkWrap<VkShaderModule> fsModule { VkEngine::getEngine().getDevice(), vkDestroyShaderModule };

	createShaderModule(VkEngine::getEngine().getDevice(), vsCode, vsModule.get());
	createShaderModule(VkEngine::getEngine().getDevice(), fsCode, fsModule.get());

	VkPipelineShaderStageCreateInfo vsStageInfo = {};
	vsStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vsStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vsStageInfo.module = vsModule;
	vsStageInfo.pName = SHADER_MAIN;

	VkPipelineShaderStageCreateInfo fsStageInfo = {};
	fsStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fsStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fsStageInfo.module = fsModule;
	fsStageInfo.pName = SHADER_MAIN;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	if (!gsPath.empty())
	{
		std::vector<char> gsCode = readFile(gsPath);
		VkWrap<VkShaderModule> gsModule { VkEngine::getEngine().getDevice(), vkDestroyShaderModule };

		createShaderModule(VkEngine::getEngine().getDevice(), gsCode, gsModule.get());

		VkPipelineShaderStageCreateInfo gsStageInfo = {};
		gsStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		gsStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
		gsStageInfo.module = gsModule;
		gsStageInfo.pName = SHADER_MAIN;

		shaderStages = { vsStageInfo, gsStageInfo, fsStageInfo };
	}
	else
	{
		shaderStages = { vsStageInfo, fsStageInfo };
	}

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.f;
	viewport.y = 0.f;
	viewport.width = (float) VkEngine::getEngine().getSwapchainExtent().width;
	viewport.height = (float) VkEngine::getEngine().getSwapchainExtent().height;
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = VkEngine::getEngine().getSwapchainExtent();

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.f;
	rasterizer.depthBiasClamp = 0.f;
	rasterizer.depthBiasSlopeFactor = 0.f;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.f;
	depthStencil.maxDepthBounds = 1.f;
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {};
	depthStencil.back = {};

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.f;
	colorBlending.blendConstants[1] = 0.f;
	colorBlending.blendConstants[2] = 0.f;
	colorBlending.blendConstants[3] = 0.f;

	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = 0;

	vkCreatePipelineLayout(VkEngine::getEngine().getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;
	pipelineInfo.pDepthStencilState = &depthStencil;

	VK_CHECK(vkCreateGraphicsPipelines(VkEngine::getEngine().getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline));
}

void RenderPass::initDepthResources()
{
	VkFormat depthFormat = findDepthFormat(VkEngine::getEngine().getPhysicalDevice());

	createImage(
		VkEngine::getEngine().getPhysicalDevice(), 
		VkEngine::getEngine().getDevice(), 
		VkEngine::getEngine().getSwapchainExtent().width, 
		VkEngine::getEngine().getSwapchainExtent().height, 
		depthFormat, 
		VK_IMAGE_TILING_OPTIMAL, 
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
		depthImage, 
		depthImageMemory);
	
	createImageView(VkEngine::getEngine().getDevice(), depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, depthImageView);

	transitionImageLayout(
		VkEngine::getEngine().getDevice(), 
		VkEngine::getEngine().getCommandPool(), 
		VkEngine::getEngine().getGraphicsQueue(), 
		depthImage, 
		VK_IMAGE_LAYOUT_UNDEFINED, 
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
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

	createBuffer(
		VkEngine::getEngine().getPhysicalDevice(), 
		VkEngine::getEngine().getDevice(), 
		bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
		uniformStagingBuffer, 
		uniformStagingBufferMemory);
	
	createBuffer(
		VkEngine::getEngine().getPhysicalDevice(), 
		VkEngine::getEngine().getDevice(), 
		bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
		uniformBuffer, 
		uniformBufferMemory);
}

void RenderPass::deleteUniformBuffers()
{
	vkDestroyBuffer(VkEngine::getEngine().getDevice(), uniformStagingBuffer, nullptr);
	vkFreeMemory(VkEngine::getEngine().getDevice(), uniformStagingBufferMemory, nullptr);
	vkDestroyBuffer(VkEngine::getEngine().getDevice(), uniformBuffer, nullptr);
	vkFreeMemory(VkEngine::getEngine().getDevice(), uniformBufferMemory, nullptr);
}

void RenderPass::deleteDepthResources()
{
	vkDestroyImage(VkEngine::getEngine().getDevice(), depthImage, nullptr);
	vkDestroyImageView(VkEngine::getEngine().getDevice(), depthImageView, nullptr);
	vkFreeMemory (VkEngine::getEngine().getDevice(), depthImageMemory, nullptr);
}

void RenderPass::deleteGraphicsPipeline()
{
	vkDestroyPipeline(VkEngine::getEngine().getDevice(), graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(VkEngine::getEngine().getDevice(), pipelineLayout, nullptr);
}

void RenderPass::deleteDescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(VkEngine::getEngine().getDevice(), descriptorSetLayout, nullptr);
}

void RenderPass::deleteRenderPass()
{
	vkDestroyRenderPass(VkEngine::getEngine().getDevice(), renderPass, nullptr);
}

void RenderPass::deleteSwapchainFramebuffers()
{
	std::vector<VkFramebuffer>::iterator it;
	for (it = swapchainFramebuffers.begin(); it != swapchainFramebuffers.end(); it++)
	{
		VkDevice device = VkEngine::getEngine().getDevice();
		vkDestroyFramebuffer(device, *it, nullptr);
	}
}