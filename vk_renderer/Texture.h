#pragma once

#include "Common.h"
#include "VkEngine.h"
#include "VkObjWrapper.h"
#include "glm\glm.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb\stb_image.h>


class Texture {
public:
	Texture(std::string path) : path(path) { initImage(); }
	~Texture() { }
	
	std::string getName() const { return name; }
	VK_WRAP(VkImageView)& getImageView() { return imageView; }
	VK_WRAP(VkSampler)& getSampler() { return sampler; }
	VK_WRAP(VkDescriptorSetLayout)& getDescriptorSetLayout() { return descriptorSetLayout; }

private:
	std::string name;
	std::string path;

	VK_WRAP(VkImage) image { VkEngine::getInstance()->getDevice(), vkDestroyImage };
	VK_WRAP(VkImageView) imageView { VkEngine::getInstance()->getDevice(), vkDestroyImageView };
	VK_WRAP(VkDeviceMemory) imageMemory { VkEngine::getInstance()->getDevice(), vkFreeMemory };
	VK_WRAP(VkSampler) sampler { VkEngine::getInstance()->getDevice(), vkDestroySampler };
	VK_WRAP(VkDescriptorSetLayout) descriptorSetLayout { VkEngine::getInstance()->getDevice(), vkDestroyDescriptorSetLayout };

	void initImage();
	void initImageView();
	void initSampler();
	void initDescriptorSetLayout();
};