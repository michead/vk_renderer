#pragma once

#include <iostream>
#include <vector>
#include "vulkan\vulkan.h"

#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYERS = false;
#else
const bool ENABLE_VALIDATION_LAYERS = true;
#endif

#define LUNARG_STANDARD_VALIDATION "VK_LAYER_LUNARG_standard_validation"
#define VK_CHECK(a) if ((a) != VK_SUCCESS) \
					throw std::runtime_error("[Vulkan] Error code: " + a);

const std::vector<const char*> validationLayers = { LUNARG_STANDARD_VALIDATION };

inline void printVulkanExtensions(const std::vector<VkExtensionProperties>& extensions)
{
	std::cout << "Available Vulkan extensions:" << std::endl;

	for (const VkExtensionProperties& extension : extensions)
	{
		std::cout << '\t' << extension.extensionName << std::endl;
	}
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
