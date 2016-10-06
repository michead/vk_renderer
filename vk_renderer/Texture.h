#pragma once

#include "VkEngine.h"


struct Texture {
public:
	Texture(std::string path) : path(path) { }
	Texture(void* pixels, unsigned int texWidth, unsigned int texHeight) : 
		pixels(pixels), texWidth(texWidth), texHeight(texHeight) { highPrec = true; initResources(); }
	~Texture() { }
	
	void init();

	std::string getName() const { return name; }
	VkImageView& getImageView() { return imageView; }
	VkSampler& getSampler() { return sampler; }

private:
	std::string name;
	std::string path;
	void* pixels;
	int texWidth, texHeight;
	bool highPrec = false;

	VkImage image;
	VkImageView imageView;
	VkDeviceMemory imageMemory;
	VkSampler sampler;

	void initResources();
};