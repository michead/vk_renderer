#define GLFW_INCLUDE_VULKAN
#include "glfw\glfw3.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "Common.h"
#include "Config.h"
#include "MathUtils.h"
#include "GeomStructs.h"
#include "VkUtils.h"
#include "VkObjWrapper.h"
#include "RenderPass.h"
#include "Scene.h"

#define APPLICATION_NAME "VkApp"

class VkApp
{
public:
	void init(int argc, char** argv);
	void run();

	static VK_WRAP(VkDevice) getDevice() { return device; }
	static VK_WRAP(VkPhysicalDevice) getPhysicalDevice() { return physicalDevice; }
	
	static VK_WRAP(VkCommandPool) getCommandPool() { return commandPool; }

	static VkQueue getGraphicsQueue() { return graphicsQueue; }
	static VkQueue getPresentationQueue() { return presentationQueue; }

	static VkFormat getSwapchainImageFormat() { return swapchainImageFormat; }
	static VkExtent2D getSwapchainExtent() { return swapchainExtent; }


	static VK_VEC_WRAP(VkImageView)& getSwapchainImageViews() { return swapchainImageViews; }
	static size_t getNumSwapchains() { return swapchainImageViews.size(); }

	static Scene& getScene() { return scene; }

	static VK_WRAP(VkSwapchainKHR)& getSwapchain() { return swapchain; }
	static VK_WRAP(VkSemaphore)& getImageAvailableSemaphore() { return imageAvailableSemaphore; }
	static VK_WRAP(VkSemaphore)& getRenderFinishedSemaphore() { return renderFinishedSemaphore; }

	static VK_WRAP(VkDescriptorPool)& getDescriptorPool() { return descriptorPool; }

private:
	VkAppConfig config;
	GLFWwindow* window;

	static VK_WRAP(VkInstance) instance;
	static VK_WRAP(VkDevice) device;
	static VK_WRAP(VkDebugReportCallbackEXT) callback;
	static VK_WRAP(VkSurfaceKHR) surface;
	static VkPhysicalDevice physicalDevice;
	static VK_WRAP(VkCommandPool) commandPool;

	static VkQueue graphicsQueue;
	static VkQueue presentationQueue;

	static VkFormat swapchainImageFormat;
	static VkExtent2D swapchainExtent;

	static VK_WRAP(VkSwapchainKHR) swapchain;
	static std::vector<VkImage> swapchainImages;
	static VK_VEC_WRAP(VkImageView) swapchainImageViews;
	
	static VK_WRAP(VkDescriptorPool) descriptorPool;

	static VK_WRAP(VkSemaphore) imageAvailableSemaphore;
	static VK_WRAP(VkSemaphore) renderFinishedSemaphore;

	std::vector<RenderPass> renderPasses;
	
	static int oldX;
	static int oldY;

	static Scene scene;

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
	void updateRenderPassData();
	void setupInputCallbacks();

	static void keyboardFunc(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouseKeyFunc(GLFWwindow* window, int button, int action, int mods);
	static void cursorPosFunc(GLFWwindow* window, double xpos, double ypos);

	static void onWindowResized(GLFWwindow* window, int width, int height);
};
