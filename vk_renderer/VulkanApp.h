#define GLFW_INCLUDE_VULKAN
#include "glfw\glfw3.h"
#include "VkUtils.h"
#include "VkObjWrapper.h"

#define APPLICATION_NAME "VkApp"

#define WINDOW_WIDTH	1280
#define WINDOW_HEIGHT	720

class VulkanApp
{
public:
	void run();
private:
	GLFWwindow* window;
	VkObjWrapper<VkInstance> instance { vkDestroyInstance };
	VkObjWrapper<VkDevice> device { vkDestroyDevice };
	VkObjWrapper<VkDebugReportCallbackEXT> callback { instance, DestroyDebugReportCallbackEXT };
	VkObjWrapper<VkSurfaceKHR> surface { instance, vkDestroySurfaceKHR };
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkQueue graphicsQueue;
	VkQueue presentationQueue;
	VkObjWrapper<VkSwapchainKHR> swapChain { device, vkDestroySwapchainKHR };
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkObjWrapper<VkImageView>> swapChainImageViews;
	VkObjWrapper<VkRenderPass> renderPass { device, vkDestroyRenderPass };
	VkObjWrapper<VkPipelineLayout> pipelineLayout { device, vkDestroyPipelineLayout };
	VkObjWrapper<VkPipeline> graphicsPipeline { device, vkDestroyPipeline };
	std::vector<VkObjWrapper<VkFramebuffer>> swapChainFramebuffers;
	VkObjWrapper<VkCommandPool> commandPool { device, vkDestroyCommandPool };
	std::vector<VkCommandBuffer> commandBuffers;
	VkObjWrapper<VkSemaphore> imageAvailableSemaphore { device, vkDestroySemaphore };
	VkObjWrapper<VkSemaphore> renderFinishedSemaphore { device, vkDestroySemaphore };

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
	void createRenderPass();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffers();
	void createSemaphores();
	void drawFrame();
	void recreateSwapChain();
	static void onWindowResized(GLFWwindow* window, int width, int height);
};
