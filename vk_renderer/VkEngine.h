#pragma once

#define GLFW_INCLUDE_VULKAN
#include "glfw\glfw3.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "Common.h"
#include "MathUtils.h"
#include "GeomStructs.h"
#include "VkUtils.h"
#include "VkObjWrapper.h"
#include "RenderPass.h"
#include "Scene.h"
#include "Config.h"

#define APPLICATION_NAME "VkEngine"

class VkEngine
{
public:
	VkEngine() { }
	~VkEngine() { if (engine) delete engine; }

	static VkEngine* engine;
	static VkEngine* getInstance() { if (engine == nullptr) engine = new VkEngine(); return engine; }

	void init(int argc, char** argv);
	void run();

	VK_WRAP(VkDevice) getDevice();
	VK_WRAP(VkPhysicalDevice) getPhysicalDevice();
	VK_WRAP(VkCommandPool) getCommandPool();
	VkQueue getGraphicsQueue();
	VkQueue getPresentationQueue();
	VkFormat getSwapchainImageFormat();
	VkExtent2D getSwapchainExtent();
	VK_VEC_WRAP(VkImageView)& getSwapchainImageViews();
	size_t getNumSwapchains();
	Scene& getScene();
	VK_WRAP(VkSwapchainKHR)& getSwapchain();
	VK_WRAP(VkSemaphore)& getImageAvailableSemaphore();
	VK_WRAP(VkSemaphore)& getRenderFinishedSemaphore();
	VK_WRAP(VkDescriptorPool)& getDescriptorPool();

	glm::ivec2 getOldMousePos() { return{ oldX, oldY }; }
	void setOldMousePos(glm::ivec2 mousePos) { oldX = mousePos.x; oldY = mousePos.y; }

private:
	VkEngineConfig config;
	GLFWwindow* window;

	VK_WRAP(VkInstance) instance { vkDestroyInstance };
	VK_WRAP(VkDevice) device { vkDestroyDevice };
	VK_WRAP(VkDebugReportCallbackEXT) callback { instance, DestroyDebugReportCallbackEXT };
	VK_WRAP(VkSurfaceKHR) surface { instance, vkDestroySurfaceKHR };
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	
	VK_WRAP(VkCommandPool) commandPool { device, vkDestroyCommandPool };
	VK_WRAP(VkDescriptorPool) descriptorPool { device, vkDestroyDescriptorPool };
	
	VK_WRAP(VkSwapchainKHR) swapchain { device, vkDestroySwapchainKHR };
	VkExtent2D swapchainExtent;
	VK_VEC_WRAP(VkImageView) swapchainImageViews;
	VK_VEC_WRAP(VkImage) swapchainImages;
	VkFormat swapchainImageFormat;
	
	VK_WRAP(VkSemaphore) imageAvailableSemaphore { device, vkDestroySemaphore };
	VK_WRAP(VkSemaphore) renderFinishedSemaphore { device, vkDestroySemaphore };

	VkQueue graphicsQueue;
	VkQueue presentationQueue;

	std::vector<RenderPass> renderPasses;
	
	int oldX;
	int oldY;

	Scene scene;

	void initWindow();
	void initVulkan();
	void mainLoop();
	void createInstance();
	void setupDebugCallback();
	void createSurface();
	void selectPhysicalDevice();
	void createLogicalDevice();
	void createSwapChain();
	void createImageViews();

	void createCommandPool();
	void createDescriptorPool();

	void createSemaphores();
	
	void initRenderPasses();

	void draw();
	void recreateSwapchain();

	void initCamera();
	void updateBufferData();
	void setupInputCallbacks();

	static void keyboardFunc(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouseKeyFunc(GLFWwindow* window, int button, int action, int mods);
	static void cursorPosFunc(GLFWwindow* window, double xpos, double ypos);

	static void onWindowResized(GLFWwindow* window, int width, int height);
};
