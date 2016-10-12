#include "VkEngine.h"

#include <chrono>
#include <iostream>
#include <set>

#include "Camera.h"
#include "Config.h"
#include "Pass.h"
#include "Scene.h"
#include "VkUtils.h"
#include "VkPool.h"
#include "GfxPipeline.h"

#include "imgui.h"
#include "imgui_impl_glfw_vulkan.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


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
	initImGui();
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

	camera->initMatrices();
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

void VkEngine::initImGui()
{
	debugCmdPool = VkEngine::getPool()->createCommandPool();

	ImGui_ImplGlfwVulkan_Init_Data init_data = {};
	init_data.allocator = nullptr;
	init_data.gpu = physicalDevice;
	init_data.device = device;
	init_data.render_pass = renderPass;
	init_data.pipeline_cache = VK_NULL_HANDLE;
	init_data.descriptor_pool = descriptorPool;
	init_data.check_vk_result = checkVkResult;
	ImGui_ImplGlfwVulkan_Init(window, true, &init_data);
	ImGui::GetIO().FontGlobalScale = (float)swapchainExtent.width / swapchainExtent.height * HUD_FONT_SCALE;
	ImGui::GetIO().DisplaySize = { swapchainExtent.width * HUD_AREA_SCALE, swapchainExtent.height * HUD_AREA_SCALE };

	debugCmdBuffers.resize(swapchainImages.size());

	for (size_t i = 0; i < swapchainImages.size(); i++)
	{
		VkCommandBufferAllocateInfo bufferAllocateInfo = {};
		bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferAllocateInfo.commandPool = debugCmdPool;
		bufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		bufferAllocateInfo.commandBufferCount = 1;
		VK_CHECK(vkAllocateCommandBuffers(device, &bufferAllocateInfo, &debugCmdBuffers[i]));
	}

	VkCommandBuffer presentCmdBuffer = debugCmdBuffers[swapchainImageIndex];

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VK_CHECK(vkBeginCommandBuffer(presentCmdBuffer, &beginInfo));

	ImGui_ImplGlfwVulkan_CreateFontsTexture(presentCmdBuffer);

	VkSubmitInfo endInfo = {};
	endInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	endInfo.commandBufferCount = 1;
	endInfo.pCommandBuffers = &presentCmdBuffer;
	VK_CHECK(vkEndCommandBuffer(presentCmdBuffer));
	VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &endInfo, VK_NULL_HANDLE));
	VK_CHECK(vkDeviceWaitIdle(device));

	ImGui_ImplGlfwVulkan_InvalidateFontUploadObjects();
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
			camera->movement = ZOOM;
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

#if SHOW_HUD
		ImGui_ImplGlfwVulkan_NewFrame();
#endif

		updateBufferData();
		draw();
	}

	VK_CHECK(vkDeviceWaitIdle(device));
	ImGui_ImplGlfwVulkan_Shutdown();
}

void VkEngine::drawDebugHUD()
{
	VK_CHECK(vkResetCommandPool(device, debugCmdPool, 0));

	VkCommandBufferBeginInfo cmdBufferBegininfo = {};
	cmdBufferBegininfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBegininfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VK_CHECK(vkBeginCommandBuffer(debugCmdBuffers[swapchainImageIndex], &cmdBufferBegininfo));

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = framebuffers[swapchainImageIndex];
	renderPassBeginInfo.renderArea.extent.width = swapchainExtent.width;
	renderPassBeginInfo.renderArea.extent.height = swapchainExtent.height;
	renderPassBeginInfo.clearValueCount = 0;

	vkCmdBeginRenderPass(debugCmdBuffers[swapchainImageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	static bool firstFrame = true;
	static std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
	static std::ofstream outFile;

	if (firstFrame)
	{
		ssaoEnabled = false;
		sssEnabled = false;
		firstFrame = false;

		outFile.open("perf.txt", std::ofstream::out | std::ofstream::trunc);
		outFile.close();
		outFile.open("perf.txt", std::ios_base::app);
	}

	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	long elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime).count();

	if (elapsed > 20000000 && !ssaoEnabled)
	{
		outFile << "SSAO" << std::endl;
		ssaoEnabled = true;
	}

	if (elapsed > 40000000 && !sssEnabled)
	{
		outFile << "SSS" << std::endl;
		sssEnabled = true;
	}

	outFile << std::to_string(elapsed * 0.000001f) << " : " << std::to_string(ImGui::GetIO().Framerate) << std::endl;

	if (std::chrono::duration_cast<std::chrono::microseconds>(now - startTime).count() > 60000000)
	{
		system("pause");
		outFile.close();
		exit(0);
	}

	ImGui::PushID("Subsurface Scattering");
	ImGui::CollapsingHeader("Subsurface Scattering");
	if (firstFrame) { ImGui::Checkbox("Toggle", &firstFrame); }
	else ImGui::Checkbox("Toggle", &sssEnabled);
	ImGui::SliderFloat("Translucency", &translucencyOverride, .0f, .9f);
	ImGui::SliderFloat("Kernel Width", &subsurfWidthOverride, 0, .05f);
	// These attributes are static in the current implementation, no way to tweak them
	// ImGui::SliderFloat3("Strength", (float*)&subsurfStrengthOverride, 0, 1);
	// ImGui::SliderFloat3("Falloff", (float*) &subsurfFalloffOverride, 0, 1);
	ImGui::PopID();

	ImGui::PushID("Ambient Occlusion");
	ImGui::CollapsingHeader("Ambient Occlusion");
	if (firstFrame) { ImGui::Checkbox("Toggle", &firstFrame); }
	else ImGui::Checkbox("Toggle", &ssaoEnabled);
	ImGui::PopID();

	ImGui::PushID("Stats");
	ImGui::CollapsingHeader("Stats");
	ImGui::Text("Avg %.3f ms/frame (%.1f FPS)", 1000.f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::PopID();

	if (firstFrame)
	{
		firstFrame = false;
	}

	ImGui_ImplGlfwVulkan_Render(debugCmdBuffers[swapchainImageIndex]);

	vkCmdEndRenderPass(debugCmdBuffers[swapchainImageIndex]);
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
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
#if SHOW_HUD
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
#else
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
#endif

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

void VkEngine::endDebugFrame()
{
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = swapchainImages[swapchainImageIndex];
	barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

	vkCmdPipelineBarrier(
		debugCmdBuffers[swapchainImageIndex],
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0,
		0,
		NULL,
		0,
		NULL,
		1,
		&barrier);

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VK_CHECK(vkEndCommandBuffer(debugCmdBuffers[swapchainImageIndex]));

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &renderCompleteSemaphore;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &debugCmdBuffers[swapchainImageIndex];
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.pSignalSemaphores = &debugDrawCompleteSemaphore;
	submitInfo.pCommandBuffers = &debugCmdBuffers[swapchainImageIndex];

	VK_CHECK(vkQueueSubmit(VkEngine::getEngine().getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
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
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &swapchainImageIndex;
	presentInfo.pResults = nullptr;
	presentInfo.waitSemaphoreCount = 1;

#if SHOW_HUD
	drawDebugHUD();
	endDebugFrame();

	presentInfo.pWaitSemaphores = &debugDrawCompleteSemaphore;
#else
	presentInfo.pWaitSemaphores = &renderCompleteSemaphore;
#endif

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
	debugDrawCompleteSemaphore = VkEngine::getEngine().getPool()->createSemaphore();
}

void VkEngine::recreateSwapchain()
{
	VK_CHECK(vkDeviceWaitIdle(device));

	delete pool;
	initPool();
	initImageViews();
	initRenderPass(); // Graphics pipeline recreation may be avoided via viewport and scissor rectangle sizes dynamic states
	initCommandPool();
	initDescriptorPool();
	initSemaphores();
	initFramebuffers();
	initOffscreenRenderPasses();
}

void VkEngine::initBufferData()
{
	gfxPipeline->initBufferData();
}

void VkEngine::updateBufferData()
{
	scene->getCamera()->updateViewMatrix();

	gfxPipeline->updateBufferData();
}

void VkEngine::initDescriptorPool()
{
	descriptorPool = VkEngine::getEngine().getPool()->createDescriptorPool(POOL_UNIFORM_BUFFER_SIZE, POOL_COMBINED_SAMPLER_SIZE);
}

void VkEngine::initOffscreenRenderPasses()
{
	gfxPipeline = new GfxPipeline();
}

void VkEngine::cleanup()
{
	delete pool;
	delete gfxPipeline;
	delete config;
	delete scene;
}