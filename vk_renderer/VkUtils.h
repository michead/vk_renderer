#pragma once

#include "vulkan\vulkan.h"
#include "VkObjWrapper.h"
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <fstream>
#include <string>

#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYERS = false;
#else
const bool ENABLE_VALIDATION_LAYERS = true;
#endif

#define S(x) #x
#define S_(x) S(x)
#define S__LINE__ S_(__LINE__)

#define LUNARG_STANDARD_VALIDATION	"VK_LAYER_LUNARG_standard_validation"
#define LUNARG_THREADING_VALIDATION "VK_LAYER_GOOGLE_threading"
#define LUNARG_PARAMETER_VALIDATION "VK_LAYER_LUNARG_parameter_validation"
#define	LUNARG_TRACKER_VALIDATION	"VK_LAYER_LUNARG_object_tracker"
#define LUNARG_IMAGE_VALIDATION		"VK_LAYER_LUNARG_image"
#define LUNARG_CORE_VALIDATION		"VK_LAYER_LUNARG_core_validation"
#define LUNARG_SWAPCHAIN_VALIDATION "VK_LAYER_LUNARG_swapchain"
#define LUNARG_UNIQUE_VALIDATION	"VK_LAYER_GOOGLE_unique_objects"

#define VK_CHECK(a) if ((a) != VK_SUCCESS) { \
						std::string errCode; \
						switch(a) { \
						case VkResult::VK_ERROR_OUT_OF_HOST_MEMORY: \
							errCode = "VK_ERROR_OUT_OF_HOST_MEMORY"; \
							break; \
						case VkResult::VK_ERROR_OUT_OF_DEVICE_MEMORY: \
							errCode = "VK_ERROR_OUT_OF_DEVICE_MEMORY"; \
							break; \
						case VkResult::VK_ERROR_DEVICE_LOST: \
							errCode = "VK_ERROR_DEVICE_LOST"; \
							break; \
						case VkResult::VK_ERROR_SURFACE_LOST_KHR: \
							errCode = "VK_ERROR_SURFACE_LOST_KHR"; \
							break; \
						case VkResult::VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: \
							errCode = "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR"; \
							break; \
						default: \
							errCode = std::to_string(a); \
							break; \
						} \
						throw std::runtime_error("[Vulkan] Error code in file " __FILE__ " at line " S__LINE__ ": " + errCode); \
					}

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentationModes;
};

struct QueueFamilyIndices
{
	int graphicsFamily = -1;
	int presentationFamily = -1;

	bool isValid()
	{
		return	graphicsFamily >= 0 && 
				presentationFamily >= 0;
	}
};

const std::vector<const char*> validationLayers = { 
	LUNARG_THREADING_VALIDATION,
	LUNARG_PARAMETER_VALIDATION,
	LUNARG_TRACKER_VALIDATION,
	LUNARG_IMAGE_VALIDATION,
	LUNARG_CORE_VALIDATION,
	LUNARG_SWAPCHAIN_VALIDATION,
	LUNARG_UNIQUE_VALIDATION
};

const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

inline std::vector<const char*> getRequiredExtensions()
{
	std::vector<const char*> extensions;

	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (unsigned int i = 0; i < glfwExtensionCount; i++)
	{
		extensions.push_back(glfwExtensions[i]);
	}

	if (ENABLE_VALIDATION_LAYERS)
	{
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	return extensions;
}

inline bool checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}

inline VkResult CreateDebugReportCallbackEXT(
	VkInstance instance, 
	const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, 
	const VkAllocationCallbacks* pAllocator, 
	VkDebugReportCallbackEXT* pCallback)
{
	auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

inline void DestroyDebugReportCallbackEXT(
	VkInstance instance, 
	VkDebugReportCallbackEXT callback, 
	const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr)
	{
		func(instance, callback, pAllocator);
	}
}

inline static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* msg,
	void* userData)
{

	std::cerr << "Validation layer: " << msg << std::endl;

	return VK_FALSE;
}

inline QueueFamilyIndices findQueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamilyCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		VkBool32 isSupportPresent = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &isSupportPresent);

		if (queueFamilyCount > 0 && isSupportPresent)
		{
			indices.presentationFamily = i;
		}

		i++;
	}

	return indices;
}

inline bool checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

inline VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && 
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

inline VkPresentModeKHR chooseSwapPresentationMode(const std::vector<VkPresentModeKHR> availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

inline VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, int preferredWidth, int preferredHeight)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent = { preferredWidth, preferredHeight };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

inline SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentationModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentationModes.data());
	}

	return details;
}

inline bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	QueueFamilyIndices indices = findQueueFamilyIndices(device, surface);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentationModes.empty();
	}

	return indices.isValid() && extensionsSupported && swapChainAdequate;
}

inline std::vector<char> readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file!");
	}

	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

inline void createShaderModule(VkDevice device, const std::vector<char>& code, VkObjWrapper<VkShaderModule>& shaderModule)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = (uint32_t*) code.data();

	VK_CHECK(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));
}
