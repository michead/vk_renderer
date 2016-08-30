#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <iostream>
#include "VkApp.h"

int main(int argc, char** argv)
{
	VkApp app;

	try
	{
		app.init(argc, argv);
		app.run();
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}