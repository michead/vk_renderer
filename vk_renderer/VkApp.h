#define GLFW_INCLUDE_VULKAN
#include "glfw\glfw3.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "Common.h"
#include "Config.h"
#include "GeomData.h"
#include "VkUtils.h"
#include "VkObjWrapper.h"

#define APPLICATION_NAME "VkApp"

#define WINDOW_WIDTH	1280
#define WINDOW_HEIGHT	720

class VkApp
{
public:
	void init(int argc, char** argv);
	void run();
private:
	VkAppConfig config;
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
	VkObjWrapper<VkDescriptorSetLayout> descriptorSetLayout { device, vkDestroyDescriptorSetLayout };
	VkObjWrapper<VkPipelineLayout> pipelineLayout { device, vkDestroyPipelineLayout };
	VkObjWrapper<VkPipeline> graphicsPipeline { device, vkDestroyPipeline };
	std::vector<VkObjWrapper<VkFramebuffer>> swapChainFramebuffers;
	VkObjWrapper<VkCommandPool> commandPool { device, vkDestroyCommandPool };
	VkObjWrapper<VkImage> textureImage { device, vkDestroyImage };
	VkObjWrapper<VkImageView> textureImageView { device, vkDestroyImageView };
	VkObjWrapper<VkSampler> textureSampler { device, vkDestroySampler };
	VkObjWrapper<VkDeviceMemory> textureImageMemory { device, vkFreeMemory };
	VkObjWrapper<VkImage> depthImage { device, vkDestroyImage };
	VkObjWrapper<VkDeviceMemory> depthImageMemory { device, vkFreeMemory };
	VkObjWrapper<VkImageView> depthImageView { device, vkDestroyImageView };
	std::vector<VkCommandBuffer> commandBuffers;
	VkObjWrapper<VkSemaphore> imageAvailableSemaphore { device, vkDestroySemaphore };
	VkObjWrapper<VkSemaphore> renderFinishedSemaphore { device, vkDestroySemaphore };
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	VkObjWrapper<VkBuffer> vertexBuffer { device, vkDestroyBuffer };
	VkObjWrapper<VkDeviceMemory> vertexBufferMemory { device, vkFreeMemory };
	VkObjWrapper<VkBuffer> indexBuffer { device, vkDestroyBuffer };
	VkObjWrapper<VkDeviceMemory> indexBufferMemory { device, vkFreeMemory };
	VkObjWrapper<VkBuffer> uniformStagingBuffer{ device, vkDestroyBuffer };
	VkObjWrapper<VkDeviceMemory> uniformStagingBufferMemory{ device, vkFreeMemory };
	VkObjWrapper<VkBuffer> uniformBuffer{ device, vkDestroyBuffer };
	VkObjWrapper<VkDeviceMemory> uniformBufferMemory{ device, vkFreeMemory };
	VkObjWrapper<VkDescriptorPool> descriptorPool { device, vkDestroyDescriptorPool };
	VkDescriptorSet descriptorSet;
	
	static Camera camera;
	static int oldX;
	static int oldY;

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
	void createDescriptorSetLayout();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createCommandPool();
	void createTextureImage();
	void createTextureImageView();
	void createTextureSampler();
	void createDepthResources();
	void loadModels();
	void createVertexBuffer();
	void createIndexBuffer();
	void createUniformBuffer();
	void createDescriptorPool();
	void createDescriptorSet();
	void createCommandBuffers();
	void createSemaphores();
	void drawFrame();
	void recreateSwapChain();
	
	static std::unordered_map<std::string, std::string> getCmdLineArgs(int argc, char** argv);

	void initCamera();
	void updateCamera();
	void setupInputCallbacks();

	static void keyboardFunc(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouseKeyFunc(GLFWwindow* window, int button, int action, int mods);
	static void cursorPosFunc(GLFWwindow* window, double xpos, double ypos);

	static void onWindowResized(GLFWwindow* window, int width, int height);
};
