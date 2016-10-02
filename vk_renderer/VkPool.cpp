#include "VkPool.h"

#include <array>

#include "VkUtils.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb\stb_image.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


VkSemaphore VkPool::createSemaphore()
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	semaphores.push_back(VK_NULL_HANDLE);
	VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphores.back()));

	return semaphores.back();
}

VkDescriptorPool VkPool::createDescriptorPool(
	uint32_t bufferDescriptorCount, 
	uint32_t imageSamplerDescriptorCount,
	uint32_t maxSets)
{
	descriptorPools.push_back(VK_NULL_HANDLE);

	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = bufferDescriptorCount;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = imageSamplerDescriptorCount;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = maxSets;

	VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPools.back()));

	return descriptorPools.back();
}

std::vector<BufferData> VkPool::createUniformBuffer(VkDeviceSize bufferSize, bool createStaging)
{
	std::vector<BufferData> bufferDataVec;

	buffers.push_back(VK_NULL_HANDLE);
	deviceMemoryList.push_back(VK_NULL_HANDLE);

	VkBufferUsageFlags usageFlags = createStaging ? VK_BUFFER_USAGE_TRANSFER_SRC_BIT : VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	createBuffer(
		physicalDevice,
		device,
		bufferSize,
		usageFlags,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		buffers.back(),
		deviceMemoryList.back());

	bufferDataVec.push_back({ buffers.back(), deviceMemoryList.back() });

	if (createStaging)
	{
		buffers.push_back(VK_NULL_HANDLE);
		deviceMemoryList.push_back(VK_NULL_HANDLE);

		createBuffer(
			physicalDevice,
			device,
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			buffers.back(),
			deviceMemoryList.back());

		bufferDataVec.push_back({ buffers.back(), deviceMemoryList.back() });
	}

	return bufferDataVec;
}

BufferData VkPool::createVertexBuffer(std::vector<Vertex> vertices)
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	vertexBuffers.push_back(VK_NULL_HANDLE);
	vertexDeviceMemoryList.push_back(VK_NULL_HANDLE);

	VkBuffer stagingVertexBuffer;
	VkDeviceMemory stagingVertexMemory;

	createBuffer(
		physicalDevice,
		device,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingVertexBuffer,
		stagingVertexMemory);

	void* data;
	VK_CHECK(vkMapMemory(device, stagingVertexMemory, 0, bufferSize, 0, &data));
	memcpy(data, vertices.data(), (size_t) bufferSize);
	vkUnmapMemory(VkEngine::getEngine().getDevice(), stagingVertexMemory);

	createBuffer(
		VkEngine::getEngine().getPhysicalDevice(),
		VkEngine::getEngine().getDevice(),
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertexBuffers.back(),
		vertexDeviceMemoryList.back());

	copyBuffer(
		VkEngine::getEngine().getDevice(),
		VkEngine::getEngine().getCommandPool(),
		VkEngine::getEngine().getGraphicsQueue(),
		stagingVertexBuffer,
		vertexBuffers.back(),
		bufferSize);

	vkDestroyBuffer(VkEngine::getEngine().getDevice(), stagingVertexBuffer, nullptr);
	vkFreeMemory(VkEngine::getEngine().getDevice(), stagingVertexMemory, nullptr);

	BufferData bufferData = {
		vertexBuffers.back(),
		vertexDeviceMemoryList.back()
	};

	return bufferData;
}

BufferData VkPool::createIndexBuffer(std::vector<uint32_t> indices)
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	indexBuffers.push_back(VK_NULL_HANDLE);
	indexDeviceMemoryList.push_back(VK_NULL_HANDLE);

	VkBuffer stagingIndexBuffer;
	VkDeviceMemory stagingIndexMemory;

	createBuffer(
		VkEngine::getEngine().getPhysicalDevice(),
		VkEngine::getEngine().getDevice(),
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingIndexBuffer,
		stagingIndexMemory);

	void* data;
	VK_CHECK(vkMapMemory(VkEngine::getEngine().getDevice(), stagingIndexMemory, 0, bufferSize, 0, &data));
	memcpy(data, indices.data(), (size_t) bufferSize);
	vkUnmapMemory(VkEngine::getEngine().getDevice(), stagingIndexMemory);

	createBuffer(
		VkEngine::getEngine().getPhysicalDevice(),
		VkEngine::getEngine().getDevice(),
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		indexBuffers.back(),
		indexDeviceMemoryList.back());

	copyBuffer(
		VkEngine::getEngine().getDevice(),
		VkEngine::getEngine().getCommandPool(),
		VkEngine::getEngine().getGraphicsQueue(),
		stagingIndexBuffer,
		indexBuffers.back(),
		bufferSize);

	vkDestroyBuffer(VkEngine::getEngine().getDevice(), stagingIndexBuffer, nullptr);
	vkFreeMemory(VkEngine::getEngine().getDevice(), stagingIndexMemory, nullptr);

	BufferData bufferData = {
		indexBuffers.back(),
		indexDeviceMemoryList.back()
	};

	return bufferData;
}

ImageData VkPool::createDepthResources()
{
	VkFormat depthFormat = findDepthFormat(physicalDevice);

	depthImages.push_back(VK_NULL_HANDLE);
	depthImageViews.push_back(VK_NULL_HANDLE);
	depthImageMemoryList.push_back(VK_NULL_HANDLE);
	depthSamplers.push_back(VK_NULL_HANDLE);

	createImage(
		physicalDevice,
		device,
		swapchainExtent.width,
		swapchainExtent.height,
		depthFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		depthImages.back(),
		depthImageMemoryList.back());

	createImageView(
		device, 
		depthImages.back(), 
		depthFormat, 
		VK_IMAGE_ASPECT_DEPTH_BIT, 
		depthImageViews.back());

	transitionImageLayout(
		VkEngine::getEngine().getDevice(),
		VkEngine::getEngine().getCommandPool(),
		VkEngine::getEngine().getGraphicsQueue(),
		depthImages.back(),
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.f;
	samplerInfo.minLod = 0.f;
	samplerInfo.maxLod = 0.f;

	VK_CHECK(vkCreateSampler(device, &samplerInfo, nullptr, &depthSamplers.back()));

	ImageData depthData = {
		depthImages.back(),
		depthImageViews.back(),
		depthImageMemoryList.back(),
		depthSamplers.back()
	};

	return depthData;
}

VkCommandPool VkPool::createCommandPool()
{
	commandPools.push_back(VK_NULL_HANDLE);

	QueueFamilyIndices queueFamilyIndices = findQueueFamilyIndices(physicalDevice, surface);

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

	VK_CHECK(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPools.back()));

	return commandPools.back();
}

PipelineData VkPool::createPipeline(
	VkRenderPass renderPass,
	VkDescriptorSetLayout descriptorSetLayout, 
	VkExtent2D extent,
	std::vector<char> vs, 
	std::vector<char> fs, 
	std::vector<char> gs,
	uint16_t numColorAttachments)
{
	pipelines.push_back(VK_NULL_HANDLE);
	pipelineLayouts.push_back(VK_NULL_HANDLE);
	
	shaderModules.push_back(VK_NULL_HANDLE);
	shaderModules.push_back(VK_NULL_HANDLE);

	size_t vsIndex = shaderModules.size() - 2;
	size_t fsIndex = shaderModules.size() - 1;

	createShaderModule(VkEngine::getEngine().getDevice(), vs, shaderModules[vsIndex]);
	createShaderModule(VkEngine::getEngine().getDevice(), fs, shaderModules[fsIndex]);

	VkPipelineShaderStageCreateInfo vsStageInfo = {};
	vsStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vsStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vsStageInfo.module = shaderModules[vsIndex];
	vsStageInfo.pName = SHADER_MAIN;

	VkPipelineShaderStageCreateInfo fsStageInfo = {};
	fsStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fsStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fsStageInfo.module = shaderModules[fsIndex];
	fsStageInfo.pName = SHADER_MAIN;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	if (!gs.empty())
	{
		shaderModules.push_back(VK_NULL_HANDLE);

		createShaderModule(device, gs, shaderModules.back());

		VkPipelineShaderStageCreateInfo gsStageInfo = {};
		gsStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		gsStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
		gsStageInfo.module = shaderModules.back();
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
	viewport.width = (float) extent.width;
	viewport.height = (float) extent.height;
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = extent;

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

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(numColorAttachments);
	for (VkPipelineColorBlendAttachmentState& colorBlendAttachment : colorBlendAttachments)
	{
		colorBlendAttachment.colorWriteMask = 0xf;
		colorBlendAttachment.blendEnable = VK_FALSE;
	}

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.attachmentCount = colorBlendAttachments.size();
	colorBlending.pAttachments = colorBlendAttachments.data();

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

	VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayouts.back()));

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = shaderStages.size();
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.layout = pipelineLayouts.back();
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;
	pipelineInfo.pDepthStencilState = &depthStencil;

	VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelines.back()));

	PipelineData pipelineData = {
		pipelines.back(),
		pipelineLayouts.back()
	};

	return pipelineData;
}

VkDescriptorSetLayout VkPool::createDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	descriptorSetLayouts.push_back(VK_NULL_HANDLE);

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = bindings.size();
	layoutInfo.pBindings = bindings.data();

	VK_CHECK(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayouts.back()));

	return descriptorSetLayouts.back();
}

VkRenderPass VkPool::createRenderPass(VkRenderPassCreateInfo createInfo)
{
	renderPasses.push_back(VK_NULL_HANDLE);

	VK_CHECK(vkCreateRenderPass(device, &createInfo, nullptr, &renderPasses.back()));

	return renderPasses.back();
}

VkFramebuffer VkPool::createFramebuffer(VkFramebufferCreateInfo createInfo)
{
	framebuffers.push_back(VK_NULL_HANDLE);

	VK_CHECK(vkCreateFramebuffer(device, &createInfo, nullptr, &framebuffers.back()));

	return framebuffers.back();
}

VkImageView VkPool::createSwapchainImageView(VkImage swapchainImage)
{
	swapchainImageViews.push_back(VK_NULL_HANDLE);

	createImageView(
		device,
		swapchainImage,
		swapchainFormat,
		VK_IMAGE_ASPECT_COLOR_BIT,
		swapchainImageViews.back());

	return swapchainImageViews.back();
}

ImageData VkPool::createTextureResources(std::string path)
{
	textureImages.push_back(VK_NULL_HANDLE);
	textureImageViews.push_back(VK_NULL_HANDLE);
	textureImageMemoryList.push_back(VK_NULL_HANDLE);
	textureSamplers.push_back(VK_NULL_HANDLE);
	
	VkImage stagingTextureImage;
	VkDeviceMemory stagingTextureImageMemory;

	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels)
	{
		throw std::runtime_error("Failed to load texture image!");
	}
	
	createImage(
		VkEngine::getEngine().getPhysicalDevice(),
		VkEngine::getEngine().getDevice(),
		texWidth,
		texHeight,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_LINEAR,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingTextureImage,
		stagingTextureImageMemory);

	void* data;
	VK_CHECK(vkMapMemory(VkEngine::getEngine().getDevice(), stagingTextureImageMemory, 0, imageSize, 0, &data));
	memcpy(data, pixels, (size_t) imageSize);
	vkUnmapMemory(VkEngine::getEngine().getDevice(), stagingTextureImageMemory);

	stbi_image_free(pixels);

	createImage(
		VkEngine::getEngine().getPhysicalDevice(),
		VkEngine::getEngine().getDevice(),
		texWidth,
		texHeight,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		textureImages.back(),
		textureImageMemoryList.back());

	transitionImageLayout(
		VkEngine::getEngine().getDevice(),
		VkEngine::getEngine().getCommandPool(),
		VkEngine::getEngine().getGraphicsQueue(),
		stagingTextureImage,
		VK_IMAGE_LAYOUT_PREINITIALIZED,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	transitionImageLayout(
		VkEngine::getEngine().getDevice(),
		VkEngine::getEngine().getCommandPool(),
		VkEngine::getEngine().getGraphicsQueue(),
		textureImages.back(),
		VK_IMAGE_LAYOUT_PREINITIALIZED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	copyImage(
		VkEngine::getEngine().getDevice(),
		VkEngine::getEngine().getCommandPool(),
		VkEngine::getEngine().getGraphicsQueue(),
		stagingTextureImage,
		textureImages.back(),
		texWidth,
		texHeight);

	transitionImageLayout(
		VkEngine::getEngine().getDevice(),
		VkEngine::getEngine().getCommandPool(),
		VkEngine::getEngine().getGraphicsQueue(),
		textureImages.back(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	createImageView(
		device,
		textureImages.back(),
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_ASPECT_COLOR_BIT,
		textureImageViews.back());

	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.f;
	samplerInfo.minLod = 0.f;
	samplerInfo.maxLod = 0.f;

	VK_CHECK(vkCreateSampler(device, &samplerInfo, nullptr, &textureSamplers.back()));

	vkDestroyImage(VkEngine::getEngine().getDevice(), stagingTextureImage, nullptr);
	vkFreeMemory(VkEngine::getEngine().getDevice(), stagingTextureImageMemory, nullptr);

	ImageData imageData = {
		textureImages.back(),
		textureImageViews.back(),
		textureImageMemoryList.back(),
		textureSamplers.back()
	};

	return imageData;
}

GBufferAttachment VkPool::createGBufferAttachment(GBufferAttachmentType type)
{
	VkFormat format;
	VkImageUsageFlagBits imageFlags;
	VkImageAspectFlagBits imageViewFlags;

	offscreenImages.push_back(VK_NULL_HANDLE);
	offscreenImageViews.push_back(VK_NULL_HANDLE);
	offscreenImageMemoryList.push_back(VK_NULL_HANDLE);
	offscreenImageSamplers.push_back(VK_NULL_HANDLE);

	switch (type)
	{
	case DEPTH:
		format = findDepthFormat(physicalDevice);
		imageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageViewFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
		break;
	case NORMAL:
	case TANGENT:
	case POSITION:
	case MATERIAL:
		format = VK_FORMAT_R16G16B16A16_SFLOAT;
		imageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		imageViewFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		break;
	case COLOR:
	case SPECULAR:
	default:
		format = VK_FORMAT_R8G8B8A8_UNORM;
		imageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		imageViewFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		break;
	}

	createImage(
		physicalDevice,
		device,
		swapchainExtent.width,
		swapchainExtent.height,
		format,
		VK_IMAGE_TILING_OPTIMAL,
		imageFlags | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		offscreenImages.back(),
		offscreenImageMemoryList.back());

	if (type == GBufferAttachmentType::DEPTH)
	{
		transitionImageLayout(
			VkEngine::getEngine().getDevice(), 
			VkEngine::getEngine().getCommandPool(),
			VkEngine::getEngine().getGraphicsQueue(),
			offscreenImages.back(),
			VK_IMAGE_LAYOUT_UNDEFINED, 
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}

	createImageView(
		device,
		offscreenImages.back(),
		format,
		imageViewFlags,
		offscreenImageViews.back());

	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.f;
	samplerInfo.minLod = 0.f;
	samplerInfo.maxLod = 0.f;

	VK_CHECK(vkCreateSampler(device, &samplerInfo, nullptr, &offscreenImageSamplers.back()));

	GBufferAttachment attachment = {
		type,
		offscreenImages.back(),
		offscreenImageViews.back(),
		offscreenImageMemoryList.back(),
		offscreenImageSamplers.back()
	};

	return attachment;
}

void VkPool::setPhysicalDevice()
{
	physicalDevice = choosePhysicalDevice(instance, surface);

	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

	std::cout << "Device in use: " << deviceProperties.deviceName << std::endl;
}

void VkPool::createSwapchain(glm::ivec2 resolution)
{
	SwapChainSupportDetails swapchainSupport = querySwapchainSupport(physicalDevice, surface);
	VkSurfaceFormatKHR surfaceFormat = pickSurfaceFormat(swapchainSupport.formats);
	VkPresentModeKHR presentMode = getPresentationMode(swapchainSupport.presentationModes);
	VkExtent2D extent = pickExtent(swapchainSupport.capabilities, resolution);

	uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
	if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount)
	{
		imageCount = swapchainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilyIndices(physicalDevice, surface);
	uint32_t queueFamilyIndices[] = { (uint32_t) indices.graphicsFamily, (uint32_t) indices.presentationFamily };

	if (indices.graphicsFamily != indices.presentationFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	// TODO: Old swapchain should be noted here
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	
	createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain);

	VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr));
	swapchainImages.resize(imageCount);
	VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data()));

	swapchainFormat = surfaceFormat.format;
	swapchainExtent = extent;
}

void VkPool::createDebugCallback()
{
	if (!ENABLE_VALIDATION_LAYERS)
		return;

	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT) debugReportCallback;

	VK_CHECK(CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &debugCallback));
}

void VkPool::createSurface(GLFWwindow* window)
{
	VK_CHECK(glfwCreateWindowSurface(instance, window, nullptr, &surface));
}

void VkPool::createDevice()
{
	QueueFamilyIndices indices = findQueueFamilyIndices(physicalDevice, surface);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentationFamily };

	float queuePriority = 1.f;
	for (int queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = (uint32_t) queueCreateInfos.size();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = deviceExtensions.size();
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (ENABLE_VALIDATION_LAYERS)
	{
		createInfo.enabledLayerCount = validationLayers.size();
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	VK_CHECK(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device));

	vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentationFamily, 0, &presentationQueue);
}

void VkPool::createInstance()
{
	if (ENABLE_VALIDATION_LAYERS && !checkValidationLayerSupport())
	{
		std::cerr << "Support for one or more requested layers is missing!" << std::endl;
	}

	auto extensions = getRequiredExtensions();

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = APPLICATION_NAME;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = ENGINE_NAME;
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();

	if (ENABLE_VALIDATION_LAYERS)
	{
		createInfo.enabledLayerCount = validationLayers.size();
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance));
}

void VkPool::freeResources()
{
	for (VkShaderModule shader : shaderModules) { vkDestroyShaderModule(device, shader, nullptr); }
	for (VkSemaphore semaphore : semaphores) { vkDestroySemaphore(device, semaphore, nullptr); }
	for (VkDescriptorPool descriptorPool : descriptorPools) { vkDestroyDescriptorPool(device, descriptorPool, nullptr); }
	for (VkDeviceMemory deviceMemory : deviceMemoryList) { vkFreeMemory(device, deviceMemory, nullptr); }
	for (VkBuffer buffer : buffers) { vkDestroyBuffer(device, buffer, nullptr); }
	for (VkDeviceMemory deviceMemory : indexDeviceMemoryList) { vkFreeMemory(device, deviceMemory, nullptr); }
	for (VkBuffer buffer : indexBuffers) { vkDestroyBuffer(device, buffer, nullptr); }
	for (VkDeviceMemory deviceMemory : vertexDeviceMemoryList) { vkFreeMemory(device, deviceMemory, nullptr); }
	for (VkBuffer buffer : vertexBuffers) { vkDestroyBuffer(device, buffer, nullptr); }
	for (VkSampler sampler : textureSamplers) { vkDestroySampler(device, sampler, nullptr); }
	for (VkImageView imageView : textureImageViews) { vkDestroyImageView(device, imageView, nullptr); }
	for (VkDeviceMemory deviceMemory : textureImageMemoryList) { vkFreeMemory(device, deviceMemory, nullptr); }
	for (VkImage image : textureImages) { vkDestroyImage(device, image, nullptr); }
	for (VkSampler sampler : depthSamplers) { vkDestroySampler(device, sampler, nullptr); }
	for (VkImage depthImage : depthImages) { vkDestroyImage(device, depthImage, nullptr); }
	for (VkImageView depthImageView : depthImageViews) { vkDestroyImageView(device, depthImageView, nullptr); }
	for (VkDeviceMemory depthImageMemory : depthImageMemoryList) { vkFreeMemory(device, depthImageMemory, nullptr); }
	for (VkSampler sampler : offscreenImageSamplers) { vkDestroySampler(device, sampler, nullptr); }
	for (VkImage image : offscreenImages) { vkDestroyImage(device, image, nullptr); }
	for (VkImageView imageView : offscreenImageViews) { vkDestroyImageView(device, imageView, nullptr); }
	for (VkDeviceMemory imageMemory : offscreenImageMemoryList) { vkFreeMemory(device, imageMemory, nullptr); }\
	for (VkPipeline pipeline : pipelines) { vkDestroyPipeline(device, pipeline, nullptr); }
	for (VkPipelineLayout pipelineLayout : pipelineLayouts) { vkDestroyPipelineLayout(device, pipelineLayout, nullptr); }
	for (VkDescriptorSetLayout descriptorSetLayout : descriptorSetLayouts) { vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr); }
	for (VkRenderPass renderPass : renderPasses) { vkDestroyRenderPass(device, renderPass, nullptr); }
	for (VkFramebuffer framebuffer : framebuffers) { vkDestroyFramebuffer(device, framebuffer, nullptr); }
	for (VkCommandPool commandPool : commandPools) { vkDestroyCommandPool(device, commandPool, nullptr); }
	for (VkImageView swapchainImageView : swapchainImageViews) { vkDestroyImageView(device, swapchainImageView, nullptr); }

	vkDestroySwapchainKHR(device, swapchain, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	destroyDebugReportCallbackEXT(instance, debugCallback, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroyInstance(instance, nullptr);
}