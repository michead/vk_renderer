#pragma once

#include <vector>

#include "VkUtils.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#define APPLICATION_NAME "VkEngine"


struct VkEngineConfig;
class RenderPass;
struct Scene;

void DestroyDebugReportCallbackEXT(
	VkInstance instance,
	VkDebugReportCallbackEXT callback,
	const VkAllocationCallbacks* pAllocator);


class VkEngine
{
public:
	VkEngine() { }
	~VkEngine() { if (engine) delete engine; }

	static VkEngine* getInstance() { if (engine == nullptr) engine = new VkEngine(); return engine; }

	void init(int argc, char** argv);
	void run();

	VkDevice& getDevice() { return device; }
	VkPhysicalDevice& getPhysicalDevice() { return physicalDevice; }
	VkCommandPool& getCommandPool() { return commandPool; }
	VkQueue& getGraphicsQueue() { return graphicsQueue; }
	VkQueue& getPresentationQueue() { return presentationQueue; }
	VkFormat& getSwapchainImageFormat() { return swapchainImageFormat; }
	VkExtent2D& getSwapchainExtent() { return swapchainExtent; }
	std::vector<VkImage>& getSwapchainImages() { return swapchainImages; }
	std::vector<VkImageView>& getSwapchainImageViews() { return swapchainImageViews; }
	VkSwapchainKHR& getSwapchain() { return swapchain; }
	VkSemaphore& getImageAvailableSemaphore() { return imageAvailableSemaphore; }
	VkSemaphore& getRenderFinishedSemaphore() { return renderFinishedSemaphore; }
	VkDescriptorPool& getDescriptorPool() { return descriptorPool; }

	Scene* getScene() { return scene; }

	glm::ivec2 getOldMousePos() { return{ oldX, oldY }; }
	void setOldMousePos(glm::ivec2 mousePos) { oldX = mousePos.x; oldY = mousePos.y; }

private:
	static VkEngine* engine;

	VkEngineConfig* config;
	GLFWwindow* window;

	VkInstance instance;
	VkDevice device;
	VkDebugReportCallbackEXT callback;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	
	VkCommandPool commandPool;
	VkDescriptorPool descriptorPool;
	
	VkSwapchainKHR swapchain;
	VkExtent2D swapchainExtent;
	std::vector<VkImageView> swapchainImageViews;
	std::vector<VkImage> swapchainImages;
	VkFormat swapchainImageFormat;
	
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;

	VkQueue graphicsQueue;
	VkQueue presentationQueue;

	std::vector<RenderPass*> renderPasses;
	
	int oldX;
	int oldY;

	Scene* scene;

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
