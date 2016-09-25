#pragma once

#include "Texture.h"

#include <array>
#include <string>

#include "VkUtils.h"
#include "VkPool.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm\glm.hpp"


void Texture::init()
{
	initResources();
	initDescriptorSetLayout();
}

void Texture::initResources()
{
	ImageData imageData = VkEngine::getEngine().getPool()->createTextureResources(path);
	image = imageData.image;
	imageView = imageData.imageView;
	imageMemory = imageData.imageMemory;
	sampler = imageData.sampler;
}

void Texture::initDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings(1);

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings[0] = samplerLayoutBinding;

	descriptorSetLayout = VkEngine::getEngine().getPool()->createDescriptorSetLayout(bindings);
}