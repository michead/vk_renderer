#include "VkPool.h"

#include <array>

#include "VkUtils.h"


VkSemaphore VkPool::createSemaphore()
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	semaphores.push_back(VK_NULL_HANDLE);
	VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphores.back()));

	return semaphores.back();
}

VkDescriptorPool VkPool::createDescriptorPool()
{
	descriptorPools.push_back(VK_NULL_HANDLE);

	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = 1;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 1;

	VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPools.back()));

	return descriptorPools.back();
}

std::array<BufferData, 2> VkPool::createUniformBuffer(size_t bufferSize)
{
	buffers.push_back(VK_NULL_HANDLE);
	buffers.push_back(VK_NULL_HANDLE);
	deviceMemoryList.push_back(VK_NULL_HANDLE);
	deviceMemoryList.push_back(VK_NULL_HANDLE);

	createBuffer(
		physicalDevice,
		device,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		buffers[buffers.size() - 2],
		deviceMemoryList[deviceMemoryList.size() - 2]);

	createBuffer(
		physicalDevice,
		device,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		buffers.back(),
		deviceMemoryList.back());

	std::array<BufferData, 2> bufferDataArray;
	bufferDataArray[0] = { buffers[buffers.size() - 2], deviceMemoryList[deviceMemoryList.size() - 2] };
	bufferDataArray[1] = { buffers.back(), deviceMemoryList.back() };

	return bufferDataArray;
}

BufferData VkPool::createVertexBuffer(std::vector<Vertex> vertices)
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	VkWrap<VkBuffer> stagingBuffer { device, vkDestroyBuffer };
	VkWrap<VkDeviceMemory> stagingBufferMemory { device, vkFreeMemory };

	vertexBuffers.push_back(VK_NULL_HANDLE);
	vertexDeviceMemoryList.push_back(VK_NULL_HANDLE);

	createBuffer(
		physicalDevice,
		device,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer.get(),
		stagingBufferMemory.get());

	void* data;
	VK_CHECK(vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data));
	memcpy(data, vertices.data(), (size_t) bufferSize);
	vkUnmapMemory(VkEngine::getEngine().getDevice(), stagingBufferMemory);

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
		stagingBuffer,
		vertexBuffers.back(),
		bufferSize);

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

	VkWrap<VkBuffer> stagingBuffer{ VkEngine::getEngine().getDevice(), vkDestroyBuffer };
	VkWrap<VkDeviceMemory> stagingBufferMemory{ VkEngine::getEngine().getDevice(), vkFreeMemory };
	createBuffer(
		VkEngine::getEngine().getPhysicalDevice(),
		VkEngine::getEngine().getDevice(),
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer.get(),
		stagingBufferMemory.get());

	void* data;
	VK_CHECK(vkMapMemory(VkEngine::getEngine().getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data));
	memcpy(data, indices.data(), (size_t) bufferSize);
	vkUnmapMemory(VkEngine::getEngine().getDevice(), stagingBufferMemory);

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
		stagingBuffer,
		indexBuffers.back(),
		bufferSize);

	BufferData bufferData = {
		indexBuffers.back(),
		indexDeviceMemoryList.back()
	};

	return bufferData;
}

DepthData VkPool::createDepthResources()
{
	VkFormat depthFormat = findDepthFormat(physicalDevice);

	depthImages.push_back(VK_NULL_HANDLE);
	depthImageViews.push_back(VK_NULL_HANDLE);
	depthImageMemoryList.push_back(VK_NULL_HANDLE);

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

	DepthData depthData = {
		depthImages.back(),
		depthImageViews.back(),
		depthImageMemoryList.back()
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
	std::vector<char> gs)
{
	pipelines.push_back(VK_NULL_HANDLE);
	pipelineLayouts.push_back(VK_NULL_HANDLE);

	VkWrap<VkShaderModule> vsModule { VkEngine::getEngine().getDevice(), vkDestroyShaderModule };
	VkWrap<VkShaderModule> fsModule { VkEngine::getEngine().getDevice(), vkDestroyShaderModule };

	createShaderModule(VkEngine::getEngine().getDevice(), vs, vsModule.get());
	createShaderModule(VkEngine::getEngine().getDevice(), fs, fsModule.get());

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

	if (!gs.empty())
	{
		VkWrap<VkShaderModule> gsModule { device, vkDestroyShaderModule };

		createShaderModule(device, gs, gsModule.get());

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

	VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayouts.back()));

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

VkDescriptorSetLayout VkPool::createDescriptorSetLayout()
{
	descriptorSetLayouts.push_back(VK_NULL_HANDLE);

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

VkImageView VkPool::createSCImageView(VkImage swapchainImage)
{
	scImageViews.push_back(VK_NULL_HANDLE);

	createImageView(
		device,
		swapchainImage,
		swapchainFormat,
		VK_IMAGE_ASPECT_COLOR_BIT,
		scImageViews.back());

	return scImageViews.back();
}

void VkPool::choosePhysicalDevice()
{
	uint32_t deviceCount = 0;
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));

	if (deviceCount == 0)
	{
		throw std::runtime_error("Failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()));

	for (const VkPhysicalDevice& device : devices)
	{
		if (isDeviceSuitable(device, surface))
		{
			physicalDevice = device;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Failed to find a suitable GPU!");
	}
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

	createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	VkSwapchainKHR oldSwapchain = swapchain;
	createInfo.oldSwapchain = oldSwapchain;

	VkSwapchainKHR newSwapChain;
	vkCreateSwapchainKHR(device, &createInfo, nullptr, &newSwapChain);

	*&swapchain = newSwapChain;

	VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr));
	scImages.resize(imageCount);
	VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, scImages.data()));

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
	createInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT) debugCallback;

	VK_CHECK(CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &debugCallback));
}

void VkPool::createSurface(GLFWwindow* window)
{
	glfwCreateWindowSurface(instance, window, nullptr, &surface);
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
		throw std::runtime_error("Requested validation layers are not available!");
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
	for (VkSemaphore semaphore : semaphores) { vkDestroySemaphore(device, semaphore, nullptr); }
	for (VkDescriptorPool descriptorPool : descriptorPools) { vkDestroyDescriptorPool(device, descriptorPool, nullptr); }
	for (VkDeviceMemory deviceMemory : deviceMemoryList) { vkFreeMemory(device, deviceMemory, nullptr); }
	for (VkBuffer buffer : buffers) { vkDestroyBuffer(device, buffer, nullptr); }
	for (VkDeviceMemory deviceMemory : indexDeviceMemoryList) { vkFreeMemory(device, deviceMemory, nullptr); }
	for (VkBuffer buffer : indexBuffers) { vkDestroyBuffer(device, buffer, nullptr); }
	for (VkDeviceMemory deviceMemory : vertexDeviceMemoryList) { vkFreeMemory(device, deviceMemory, nullptr); }
	for (VkBuffer buffer : vertexBuffers) { vkDestroyBuffer(device, buffer, nullptr); }
	for (VkImage depthImage : depthImages) { vkDestroyImage(device, depthImage, nullptr); }
	for (VkImageView depthImageView : depthImageViews) { vkDestroyImageView(device, depthImageView, nullptr); }
	for (VkDeviceMemory depthImageMemory : depthImageMemoryList) { vkDestroyImage(device, depthImageMemory, nullptr); }
	for (VkCommandPool commandPool : commandPools) { vkDestroyCommandPool(device, commandPool, nullptr); }
	for (VkPipeline pipeline : pipelines) { vkDestroyPipeline(device, pipeline, nullptr); }
	for (VkPipelineLayout pipelineLayout : pipelineLayouts) { vkDestroyPipelineLayout(device, pipelineLayout, nullptr); }
	for (VkDescriptorSetLayout descriptorSetLayout : descriptorSetLayouts) { vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr); }
	for (VkRenderPass renderPass : renderPasses) { vkDestroyRenderPass(device, renderPass, nullptr); }
	for (VkFramebuffer framebuffer : framebuffers) { vkDestroyFramebuffer(device, framebuffer, nullptr); }
	for (VkImageView scImageView : scImageViews) { vkDestroyImageView(device, scImageView, nullptr); }
}