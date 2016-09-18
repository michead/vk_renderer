#include "VkEngine.h"

#include <set>

#include "Camera.h"
#include "Config.h"
#include "RenderPass.h"
#include "Scene.h"
#include "VkUtils.h"
#include "VkPool.h"


void VkEngine::init(int argc, char** argv)
{
	config = new VkEngineConfig();
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

void VkEngine::initVulkan()
{
	initPool();
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
	imageAvailableSemaphore = VkEngine::getEngine().getPool()->createSemaphore();
	renderFinishedSemaphore = VkEngine::getEngine().getPool()->createSemaphore();
}

void VkEngine::recreateSwapchain()
{
	VK_CHECK(vkDeviceWaitIdle(device));

	// TODO
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
	descriptorPool = VkEngine::getEngine().getPool()->createDescriptorPool();
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
	delete pool;
	delete config;
	delete scene;
}