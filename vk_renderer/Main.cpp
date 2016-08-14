#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <iostream>
#include "RenderLoop.h"

int main()
{
	RenderLoop loop;

	try
	{
		loop.run();
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}