#pragma once

#include <vector>

#include "VkUtils.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#define APPLICATION_NAME "VkEngine"
#define ENGINE_NAME APPLICATION_NAME

#define SHADER_MAIN "main"
#define SHADER_PATH	"shaders/"

struct VkEngineConfig;
class RenderPass;
struct Scene;

enum ShadingModel {
	DEFAULT,
	NUM_SHADING_MODELS
};


class VkEngine
{
public:
	VkEngine() { }
	~VkEngine() { cleanup(); }

	static VkEngine& getEngine() { static VkEngine engine; return engine; }
	static void init(int argc, char** argv);
	static void run();

	VkInstance& getInstance() { return instance.get(); }
	VkDevice& getDevice() { return device.get(); }
	VkPhysicalDevice& getPhysicalDevice() { return physicalDevice; }
	VkSurfaceKHR& getSurface() { return surface.get(); }
	VkCommandPool& getCommandPool() { return commandPool.get(); }
	VkQueue& getGraphicsQueue() { return graphicsQueue; }
	VkQueue& getPresentationQueue() { return presentationQueue; }
	VkFormat& getSwapchainImageFormat() { return swapchainImageFormat; }
	VkExtent2D& getSwapchainExtent() { return swapchainExtent; }
	std::vector<VkImage>& getSwapchainImages() { return swapchainImages; }
	std::vector<VkImageView>& getSwapchainImageViews() { return swapchainImageViews; }
	VkSwapchainKHR& getSwapchain() { return swapchain.get(); }
	VkSemaphore& getImageAvailableSemaphore() { return imageAvailableSemaphore.get(); }
	VkSemaphore& getRenderFinishedSemaphore() { return renderFinishedSemaphore.get(); }
	VkDescriptorPool& getDescriptorPool() { return descriptorPool.get(); }

	uint32_t getImageIndex() const { return imageIndex; }

	Scene* getScene() { return scene; }

	glm::ivec2 getOldMousePos() { return{ oldX, oldY }; }
	void setOldMousePos(glm::ivec2 mousePos) { oldX = mousePos.x; oldY = mousePos.y; }

private:
	VkEngineConfig* config;
	GLFWwindow* window;

	VkWrap<VkInstance> instance { vkDestroyInstance };
	VkWrap<VkDevice> device { vkDestroyDevice };
	VkWrap<VkDebugReportCallbackEXT> callback { instance, DestroyDebugReportCallbackEXT };
	VkWrap<VkSurfaceKHR> surface { instance, vkDestroySurfaceKHR };
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	
	VkWrap<VkCommandPool> commandPool { device, vkDestroyCommandPool };
	VkWrap<VkDescriptorPool> descriptorPool { device, vkDestroyDescriptorPool };
	
	VkWrap<VkSwapchainKHR> swapchain { device, vkDestroySwapchainKHR };
	VkExtent2D swapchainExtent;
	std::vector<VkImageView> swapchainImageViews;
	std::vector<VkImage> swapchainImages;
	VkFormat swapchainImageFormat;
	
	VkWrap<VkSemaphore> imageAvailableSemaphore { device, vkDestroySemaphore };
	VkWrap<VkSemaphore> renderFinishedSemaphore { device, vkDestroySemaphore };

	VkQueue graphicsQueue;
	VkQueue presentationQueue;

	uint32_t imageIndex;

	std::vector<RenderPass*> renderPasses;
	
	int oldX;
	int oldY;

	Scene* scene;

	void initWindow();
	void initVulkan();
	void mainLoop();
	void initInstance();
	void setupDebugCallback();
	void initSurface();
	void selectPhysicalDevice();
	void initLogicalDevice();
	void initSwapchain();
	void initImageViews();

	void initCommandPool();
	void initDescriptorPool();

	void initSemaphores();
	
	void loadScene();
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

	void cleanup();
};
