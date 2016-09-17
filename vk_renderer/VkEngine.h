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

class VkPool;


class VkEngine
{
public:
	VkEngine() { }
	~VkEngine() { cleanup(); }

	static VkEngine& getEngine() { static VkEngine engine; return engine; }
	static void init(int argc, char** argv);
	static void run();

	VkInstance getInstance() { return instance; }
	VkDevice getDevice() { return device; }
	VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
	VkSurfaceKHR getSurface() { return surface; }
	VkCommandPool getCommandPool() { return commandPool; }
	VkQueue getGraphicsQueue() { return graphicsQueue; }
	VkQueue getPresentationQueue() { return presentationQueue; }
	VkFormat getSwapchainFormat() { return swapchainFormat; }
	VkExtent2D& getSwapchainExtent() { return swapchainExtent; }
	std::vector<VkImage>& getSwapchainImages() { return swapchainImages; }
	std::vector<VkImageView>& getSwapchainImageViews() { return swapchainImageViews; }
	VkSwapchainKHR& getSwapchain() { return swapchain; }
	VkSemaphore getImageAvailableSemaphore() { return imageAvailableSemaphore; }
	VkSemaphore getRenderFinishedSemaphore() { return renderFinishedSemaphore; }
	VkDescriptorPool getDescriptorPool() { return descriptorPool; }
	uint32_t getSwapchainImageIndex() const { return swapchainImageIndex; }
	VkEngineConfig* getConfig() const { return config; }
	Scene* getScene() const { return scene; }
	VkPool* getPool() const { return pool; }

	glm::ivec2 getOldMousePos() { return{ oldX, oldY }; }
	void setOldMousePos(glm::ivec2 mousePos) { oldX = mousePos.x; oldY = mousePos.y; }

private:
	VkEngineConfig* config;
	GLFWwindow* window;
	VkPool* pool;

	VkInstance instance;
	VkDebugReportCallbackEXT debugCallback;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	VkCommandPool commandPool;
	VkDescriptorPool descriptorPool;
	VkSwapchainKHR swapchain;
	VkExtent2D swapchainExtent;
	std::vector<VkImageView> swapchainImageViews;
	std::vector<VkImage> swapchainImages;
	VkFormat swapchainFormat;
	VkQueue graphicsQueue;
	VkQueue presentationQueue;
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;

	uint32_t swapchainImageIndex;

	std::vector<RenderPass*> renderPasses;
	
	int oldX;
	int oldY;

	Scene* scene;

	void initWindow();
	void initVulkan();
	void mainLoop();
	void initPool();
	void initImageViews();
	void initCommandPool();
	void initDescriptorPool();
	void initSemaphores();
	void loadScene();
	void initCamera();
	void initRenderPasses();
	void draw();
	void recreateSwapchain();
	void updateBufferData();
	void setupInputCallbacks();

	static void keyboardFunc(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouseKeyFunc(GLFWwindow* window, int button, int action, int mods);
	static void cursorPosFunc(GLFWwindow* window, double xpos, double ypos);

	static void onWindowResized(GLFWwindow* window, int width, int height);

	void cleanup();
};
