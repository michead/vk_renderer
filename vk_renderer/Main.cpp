#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <iostream>
#include "VulkanApp.h"

int main()
{
	VulkanApp app;

	try
	{
		app.run();
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}