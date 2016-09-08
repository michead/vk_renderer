#pragma once

#include "VkEngine.h"


struct Texture {
public:
	Texture(std::string path) : path(path) { init(); }
	~Texture() { }
	
	std::string getName() const { return name; }
	VkImageView& getImageView() { return imageView.get(); }
	VkSampler& getSampler() { return sampler.get(); }
	VkDescriptorSetLayout& getDescriptorSetLayout() { return descriptorSetLayout.get(); }

private:
	std::string name;
	std::string path;

	VkWrap<VkImage> image { VkEngine::getEngine().getDevice(), vkDestroyImage };
	VkWrap<VkImageView> imageView { VkEngine::getEngine().getDevice(), vkDestroyImageView };
	VkWrap<VkDeviceMemory> imageMemory { VkEngine::getEngine().getDevice(), vkFreeMemory };
	VkWrap<VkSampler> sampler { VkEngine::getEngine().getDevice(), vkDestroySampler };
	VkWrap<VkDescriptorSetLayout> descriptorSetLayout { VkEngine::getEngine().getDevice(), vkDestroyDescriptorSetLayout };

	void init();
	void initImage();
	void initImageView();
	void initSampler();
	void initDescriptorSetLayout();
};