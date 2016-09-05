#pragma once

#include "VkEngine.h"


struct Texture {
public:
	Texture(std::string path) : path(path) { initImage(); }
	~Texture() { }
	
	std::string getName() const { return name; }
	VkImageView& getImageView() { return imageView.get(); }
	VkSampler& getSampler() { return sampler.get(); }
	VkDescriptorSetLayout& getDescriptorSetLayout() { return descriptorSetLayout.get(); }

private:
	std::string name;
	std::string path;

	VkWrap<VkImage> image { VkEngine::getInstance().getDevice(), vkDestroyImage };
	VkWrap<VkImageView> imageView { VkEngine::getInstance().getDevice(), vkDestroyImageView };
	VkWrap<VkDeviceMemory> imageMemory { VkEngine::getInstance().getDevice(), vkFreeMemory };
	VkWrap<VkSampler> sampler { VkEngine::getInstance().getDevice(), vkDestroySampler };
	VkWrap<VkDescriptorSetLayout> descriptorSetLayout { VkEngine::getInstance().getDevice(), vkDestroyDescriptorSetLayout };

	void initImage();
	void initImageView();
	void initSampler();
	void initDescriptorSetLayout();
};