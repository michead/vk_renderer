#pragma once

#include "VkEngine.h"


struct Texture {
public:
	Texture(std::string path) : path(path) { /*init();*/ }
	~Texture() { }
	
	void init();

	std::string getName() const { return name; }
	VkImageView& getImageView() { return imageView; }
	VkSampler& getSampler() { return sampler; }
	VkDescriptorSetLayout getDescriptorSetLayout() { return descriptorSetLayout; }

private:
	std::string name;
	std::string path;

	VkImage image;
	VkImageView imageView;
	VkDeviceMemory imageMemory;
	VkSampler sampler;
	VkDescriptorSetLayout descriptorSetLayout;

	void initResources();
	void initDescriptorSetLayout();
};