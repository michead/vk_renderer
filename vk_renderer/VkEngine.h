#pragma once

#include <array>
#include <vector>

#include "GBuffer.h"
#include "VkUtils.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#define APPLICATION_NAME "VkEngine"
#define ENGINE_NAME APPLICATION_NAME
#define SHADER_MAIN "main"
#define SHADER_PATH	"shaders/"
#define NUM_GBUFFERS	3

struct Config;
struct Scene;
class VkPool;
class GfxPipeline;


class VkEngine
{
public:
	VkEngine() { }
	~VkEngine() { cleanup(); }

	static VkEngine& getEngine() { static VkEngine engine; return engine; }
	void init(int argc, char** argv);
	void run();

	VkInstance getInstance() { return instance; }
	VkDevice getDevice() { return device; }
	VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
	VkSurfaceKHR getSurface() { return surface; }
	VkCommandPool getCommandPool() { return commandPool; }
	VkQueue getGraphicsQueue() { return graphicsQueue; }
	VkQueue getPresentationQueue() { return presentationQueue; }
	VkFormat getSwapchainFormat() { return swapchainFormat; }
	VkExtent2D getSwapchainExtent() { return swapchainExtent; }
	std::vector<VkImage>& getSwapchainImages() { return swapchainImages; }
	std::vector<VkImageView>& getSwapchainImageViews() { return swapchainImageViews; }
	std::vector<VkFramebuffer>& getSwapchainFramebuffers() { return framebuffers; }
	VkSwapchainKHR getSwapchain() { return swapchain; }
	VkSemaphore getImageAvailableSemaphore() { return imageAvailableSemaphore; }
	VkDescriptorPool getDescriptorPool() { return descriptorPool; }
	VkDescriptorSetLayout getOneStageDescriptorSetLayout() { return oneStageDescriptorSetLayout; }
	VkDescriptorSetLayout getTwoStageDescriptorSetLayout() { return twoStageDescriptorSetLayout; }
	uint32_t getSwapchainImageIndex() const { return swapchainImageIndex; }
	Config* getConfig() const { return config; }
	Scene* getScene() const { return scene; }
	VkPool* getPool() const { return pool; }
	GBuffer* getCurrentGBuffer() { return &gBuffers[gBufferIndex]; }
	GBuffer* getNextGBuffer() { gBufferIndex++; gBufferIndex %= gBuffers.size(); return &gBuffers[gBufferIndex]; }
	std::array<GBuffer, NUM_GBUFFERS>& getGBuffers() { return gBuffers; }

	glm::ivec2 getOldMousePos() { return{ oldX, oldY }; }
	void setOldMousePos(glm::ivec2 mousePos) { oldX = mousePos.x; oldY = mousePos.y; }

private:
	VkPool* pool;
	Config* config;
	GLFWwindow* window;

	VkInstance instance;
	VkDebugReportCallbackEXT debugCallback;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	VkCommandPool commandPool;
	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout oneStageDescriptorSetLayout;
	VkDescriptorSetLayout twoStageDescriptorSetLayout;
	VkSwapchainKHR swapchain;
	VkExtent2D swapchainExtent;
	std::vector<VkImageView> swapchainImageViews;
	std::vector<VkImage> swapchainImages;
	VkRenderPass renderPass;
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
	std::vector<VkFramebuffer> framebuffers;
	VkFormat swapchainFormat;
	VkQueue graphicsQueue;
	VkQueue presentationQueue;
	VkSemaphore imageAvailableSemaphore;

	uint32_t swapchainImageIndex;
	uint16_t gBufferIndex = 0;
	std::array<GBuffer, NUM_GBUFFERS> gBuffers;
	GfxPipeline* gfxPipeline;

	int oldX;
	int oldY;

	Scene* scene;

	void initWindow();
	void initVulkan();
	void mainLoop();
	void initPool();
	void initDescriptorSetLayouts();
	void initImageViews();
	void initRenderPass();
	void initDepthResources();
	void initFramebuffers();
	void initCommandPool();
	void initDescriptorPool();
	void initGBuffers();
	void initSemaphores();
	void loadScene();
	void initCamera();
	void initOffscreenRenderPasses();

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
