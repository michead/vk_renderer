#include "VkEngine.h"

#include <set>

#include "Camera.h"
#include "Config.h"
#include "RenderPass.h"
#include "Scene.h"
#include "VkUtils.h"


void VkEngine::init(int argc, char** argv)
{
	getEngine().config = new VkEngineConfig();
	getEngine().config->parseCmdLineArgs(argc, argv);
}

void VkEngine::run()
{
	getEngine().loadScene();
	getEngine().initWindow();
	getEngine().initVulkan();
	getEngine().initCamera();
	getEngine().setupInputCallbacks();
	getEngine().mainLoop();
}

void VkEngine::loadScene()
{
	scene = new Scene(config->scenePath);
}

void VkEngine::initCamera()
{
	Camera* camera = scene->getCamera();
	camera->aspectRatio = swapchainExtent.width / (float) swapchainExtent.height;
	camera->movement = STILL;
}

void VkEngine::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(config->resolution.x, config->resolution.y, APPLICATION_NAME, nullptr, nullptr);

	glfwSetWindowUserPointer(window, this);
	glfwSetWindowSizeCallback(window, onWindowResized);
}

void VkEngine::setupInputCallbacks()
{
	glfwSetKeyCallback(window, keyboardFunc);
	glfwSetMouseButtonCallback(window, mouseKeyFunc);
	glfwSetCursorPosCallback(window, cursorPosFunc);
}

void VkEngine::keyboardFunc(GLFWwindow* window, int key, int scancode, int action, int mods)
{

}

void VkEngine::mouseKeyFunc(GLFWwindow* window, int button, int action, int mods)
{
	Camera* camera = getEngine().getScene()->getCamera();
	
	if (action == GLFW_RELEASE)
	{
		camera->movement = STILL;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (mods & GLFW_MOD_ALT)
		{
			camera->movement = ROTATE;
		}
		else
		{
			camera->movement = ROTATE_AROUND_TARGET;
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		camera->movement = PAN;
	}
}

void VkEngine::cursorPosFunc(GLFWwindow* window, double xpos, double ypos)
{
	glm::ivec2 oldMousePos = getEngine().getOldMousePos();
	glm::vec2 deltaPos = { xpos - oldMousePos.x, ypos - oldMousePos.y };
	getEngine().setOldMousePos({ xpos, ypos });

	Camera* camera = getEngine().getScene()->getCamera();

	switch (camera->movement)
	{
	case ROTATE:
		camera->rotate(CAMERA_ROT_SCALE * deltaPos);
		break;
	case ROTATE_AROUND_TARGET:
		camera->rotateAroundTarget(CAMERA_ROT_SCALE * deltaPos);
		break;
	case PAN:
		camera->pan(CAMERA_PAN_SCALE * deltaPos);
		break;
	case ZOOM:
		camera->zoom(CAMERA_DOLLY_SCALE * deltaPos.y);
		break;
	case STILL:
	default:
		break;
	}
}

void VkEngine::onWindowResized(GLFWwindow* window, int width, int height)
{
	if (width == 0 || height == 0) return;

	VkEngine* engine = reinterpret_cast<VkEngine*>(glfwGetWindowUserPointer(window));
	engine->recreateSwapchain();
}

void VkEngine::initInstance()
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

void VkEngine::setupDebugCallback()
{
	if (!ENABLE_VALIDATION_LAYERS)
		return;

	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT) debugCallback;

	VK_CHECK(CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback));
}

void VkEngine::initSurface()
{
	glfwCreateWindowSurface(instance, window, nullptr, &surface);
}

void VkEngine::initVulkan()
{
	initInstance();
	setupDebugCallback();
	initSurface();
	selectPhysicalDevice();
	initLogicalDevice();
	initSwapchain();
	initImageViews();
	initCommandPool();
	initDescriptorPool();
	initSemaphores();
	initRenderPasses();
}

void VkEngine::mainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		updateBufferData();
		draw();
	}

	VK_CHECK(vkDeviceWaitIdle(device));
}

void VkEngine::selectPhysicalDevice()
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

void VkEngine::initLogicalDevice()
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

void VkEngine::initSwapchain()
{
	SwapChainSupportDetails swapchainSupport = querySwapchainSupport(physicalDevice, surface);
	VkSurfaceFormatKHR surfaceFormat = pickSurfaceFormat(swapchainSupport.formats);
	VkPresentModeKHR presentMode = getPresentationMode(swapchainSupport.presentationModes);
	VkExtent2D extent = pickExtent(swapchainSupport.capabilities, config->resolution);

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
	swapchainImages.resize(imageCount);
	VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data()));

	swapchainImageFormat = surfaceFormat.format;
	swapchainExtent = extent;
}

void VkEngine::initImageViews()
{
	swapchainImageViews.resize(swapchainImages.size());

	for (uint32_t i = 0; i < swapchainImages.size(); i++)
	{
		createImageView(
			device, 
			swapchainImages[i], 
			swapchainImageFormat, 
			VK_IMAGE_ASPECT_COLOR_BIT, 
			swapchainImageViews[i]);
	}
}

void VkEngine::initCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilyIndices(physicalDevice, surface);

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

	VK_CHECK(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool));
}

void VkEngine::draw()
{
	VkResult result = vkAcquireNextImageKHR(
		device, 
		swapchain, 
		std::numeric_limits<uint64_t>::max(), 
		imageAvailableSemaphore, 
		VK_NULL_HANDLE, 
		&swapchainImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapchain();
		return;
	}
	else
	{
		VK_CHECK(result);
	}
	
	for (RenderPass* renderPass : renderPasses)
	{
		VkResult result = renderPass->run();

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			recreateSwapchain();
		}
		else
		{
			VK_CHECK(result);
		}
	}
}

void VkEngine::initSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore));
	VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore));
}

void VkEngine::recreateSwapchain()
{
	VK_CHECK(vkDeviceWaitIdle(device));

	initSwapchain();
	initImageViews();
	initRenderPasses();
}

void VkEngine::updateBufferData()
{
	scene->getCamera()->updateMatrices();

	for (RenderPass* renderPass : renderPasses)
	{
		renderPass->updateData();
	}
}

void VkEngine::initDescriptorPool()
{
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

	VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool));
}

void VkEngine::initRenderPasses()
{
	switch (config->shadingModel)
	{
	case ShadingModel::DEFAULT:
	default:
		renderPasses.push_back(new RenderPass(SHADER_PATH"vert.spv", SHADER_PATH"frag.spv"));
		break;
	}

	for each(RenderPass* renderPass in renderPasses)
		renderPass->init();
}

void VkEngine::cleanup()
{
	vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
	vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);

	for each(RenderPass* renderPass in renderPasses)
	{
		renderPass->deleteUniformBuffers();
	}

	delete scene;
	delete config;

	for each(RenderPass* renderPass in renderPasses)
	{
		renderPass->deleteDepthResources();
	}

	vkDestroyCommandPool(device, commandPool, nullptr);

	for each(RenderPass* renderPass in renderPasses)
	{
		renderPass->deleteGraphicsPipeline();
		renderPass->deleteDescriptorSetLayout();
		renderPass->deleteRenderPass();
	}	

	for each(auto& swapchainImageView in swapchainImageViews)
	{
		vkDestroyImageView(device, swapchainImageView, nullptr);
	}

	for each(RenderPass* renderPass in renderPasses)
	{
		renderPass->deleteSwapchainFramebuffers();
		delete renderPass;
	}

	vkDestroySwapchainKHR(device, swapchain, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	destroyDebugReportCallbackEXT(instance, callback, nullptr);
	vkDestroyInstance(instance, nullptr);
}