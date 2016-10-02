#include "VkEngine.h"

#include <set>

#include "Camera.h"
#include "Config.h"
#include "Pass.h"
#include "Scene.h"
#include "VkUtils.h"
#include "VkPool.h"
#include "GfxPipeline.h"


void VkEngine::init(int argc, char** argv)
{
	config = new Config();
	config->parseCmdLineArgs(argc, argv);
}

void VkEngine::run()
{
	loadScene();
	initWindow();
	initVulkan();
	initCamera();
	setupInputCallbacks();
	mainLoop();
}

void VkEngine::loadScene()
{
	scene = new Scene(config->scenePath);
}

void VkEngine::initCamera()
{
	Camera* camera = scene->getCamera();
	assert(camera);
	
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
		if (mods & GLFW_MOD_ALT)
		{
			camera->movement = PAN;
		}
		else
		{
			// TODO: Other camera movement mode?
		}
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

void VkEngine::initVulkan()
{
	initPool();
	initImageViews();
	initRenderPass();
	initCommandPool();
	initDescriptorPool();
	initSemaphores();
	initFramebuffers();
	initOffscreenRenderPasses();
}

void VkEngine::mainLoop()
{
	initBufferData();

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		updateBufferData();
		draw();
	}

	VK_CHECK(vkDeviceWaitIdle(device));
}

void VkEngine::initPool()
{
	pool = new VkPool(window, config);
	
	instance = pool->getInstance();
	device = pool->getDevice();
	physicalDevice = pool->getPhysicalDevice();
	surface = pool->getSurface();
	debugCallback = pool->getDebugCallback();
	swapchain = pool->getSwapchain();
	graphicsQueue = pool->getGraphicsQueue();
	presentationQueue = pool->getPresentationQueue();
	swapchainImages = pool->getSwapchainImages();
	swapchainFormat = pool->getSwapchainFormat();
	swapchainExtent = pool->getSwapchainExtent();
}

void VkEngine::initImageViews()
{
	swapchainImageViews.resize(swapchainImages.size());

	for (uint32_t i = 0; i < swapchainImages.size(); i++)
	{
		swapchainImageViews[i] = VkEngine::getEngine().getPool()->createSwapchainImageView(swapchainImages[i]);
	}
}

void VkEngine::initRenderPass()
{
	VkAttachmentDescription attachment = {};
	attachment.format = swapchainFormat;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorReference;
	subpassDescription.pDepthStencilAttachment = nullptr;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;
	subpassDescription.pResolveAttachments = nullptr;

	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &attachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = dependencies.size();
	renderPassInfo.pDependencies = dependencies.data();

	renderPass = pool->createRenderPass(renderPassInfo);
}

void VkEngine::initFramebuffers()
{
	framebuffers.resize(swapchainImageViews.size());

	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderPass;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.width = VkEngine::getEngine().getSwapchainExtent().width;
	framebufferInfo.height = VkEngine::getEngine().getSwapchainExtent().height;
	framebufferInfo.layers = 1;

	for (size_t i = 0; i < swapchainImageViews.size(); i++)
	{
		framebufferInfo.pAttachments = &swapchainImageViews[i];
		framebuffers[i] = VkEngine::getEngine().getPool()->createFramebuffer(framebufferInfo);
	}
}

void VkEngine::initCommandPool()
{
	commandPool = VkEngine::getEngine().getPool()->createCommandPool();
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

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		recreateSwapchain();
		return;
	}
	
	gfxPipeline->run();

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderCompleteSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &swapchainImageIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(VkEngine::getEngine().getPresentationQueue(), &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		recreateSwapchain();
	}
}

void VkEngine::initSemaphores()
{
	imageAvailableSemaphore = VkEngine::getEngine().getPool()->createSemaphore();
	renderCompleteSemaphore = VkEngine::getEngine().getPool()->createSemaphore();
}

void VkEngine::recreateSwapchain()
{
	VK_CHECK(vkDeviceWaitIdle(device));

	// TODO
}

void VkEngine::initBufferData()
{
	gfxPipeline->initBufferData();
}

void VkEngine::updateBufferData()
{
	scene->getCamera()->updateMatrices();

	gfxPipeline->updateBufferData();
}

void VkEngine::initDescriptorPool()
{
	uint16_t numPasses = gfxPipeline->getNumPasses();
	descriptorPool = VkEngine::getEngine().getPool()->createDescriptorPool(numPasses * 5, numPasses * 5); // Rough estimate
}

void VkEngine::initOffscreenRenderPasses()
{
	gfxPipeline = new GfxPipeline();
}

void VkEngine::cleanup()
{
	delete gfxPipeline;
	delete pool;
	delete config;
	delete scene;
}