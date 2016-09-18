#include <iostream>

#include "VkEngine.h"


int main(int argc, char** argv)
{
	try
	{
		VkEngine::getEngine().init(argc, argv);
		VkEngine::getEngine().run();
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}