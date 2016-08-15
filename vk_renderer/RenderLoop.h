#define GLFW_INCLUDE_VULKAN
#include "glfw\glfw3.h"
#include "VkUtils.h"
#include "VkObjWrapper.h"

#define WINDOW_WIDTH	1280
#define WINDOW_HEIGHT	720

class RenderLoop
{
public:
	void run();
private:
	GLFWwindow* window;
	VkObjWrapper<VkInstance> instance { vkDestroyInstance };
	VkObjWrapper<VkDevice> device{ vkDestroyDevice };
	VkObjWrapper<VkDebugReportCallbackEXT> callback{ instance, DestroyDebugReportCallbackEXT };
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkQueue graphicsQueue;

	void initWindow();
	void initVulkan();
	void mainLoop();
	void createInstance();
	void setupDebugCallback();
	void selectPhysicalDevice();
	void createLogicalDevice();
};
