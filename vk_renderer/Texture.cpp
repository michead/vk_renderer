#pragma once

#include "Texture.h"


void Texture::initImage()
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels)
	{
		throw std::runtime_error("Failed to load texture image!");
	}

	VK_WRAP(VkImage) stagingImage { VkEngine::getInstance()->getDevice(), vkDestroyImage };
	VK_WRAP(VkDeviceMemory) stagingImageMemory { VkEngine::getInstance()->getDevice(), vkFreeMemory };
	createImage(
		VkEngine::getInstance()->getPhysicalDevice(),
		VkEngine::getInstance()->getDevice(), texWidth,
		texHeight, 
		VK_FORMAT_R8G8B8A8_UNORM, 
		VK_IMAGE_TILING_LINEAR, 
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
		stagingImage, 
		stagingImageMemory);

	void* data;
	vkMapMemory(VkEngine::getInstance()->getDevice(), stagingImageMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, (size_t) imageSize);
	vkUnmapMemory(VkEngine::getInstance()->getDevice(), stagingImageMemory);

	stbi_image_free(pixels);

	createImage(
		VkEngine::getInstance()->getPhysicalDevice(), 
		VkEngine::getInstance()->getDevice(), 
		texWidth, 
		texHeight, 
		VK_FORMAT_R8G8B8A8_UNORM, 
		VK_IMAGE_TILING_OPTIMAL, 
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
		image, 
		imageMemory);

	transitionImageLayout(
		VkEngine::getInstance()->getDevice(), 
		VkEngine::getInstance()->getCommandPool(), 
		VkEngine::getInstance()->getGraphicsQueue(), 
		stagingImage, 
		VK_IMAGE_LAYOUT_PREINITIALIZED, 
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	
	transitionImageLayout(
		VkEngine::getInstance()->getDevice(), 
		VkEngine::getInstance()->getCommandPool(), 
		VkEngine::getInstance()->getGraphicsQueue(), 
		image, 
		VK_IMAGE_LAYOUT_PREINITIALIZED, 
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	
	copyImage(
		VkEngine::getInstance()->getDevice(), 
		VkEngine::getInstance()->getCommandPool(),
		VkEngine::getInstance()->getGraphicsQueue(),
		stagingImage, 
		image, 
		texWidth, 
		texHeight);

	transitionImageLayout(
		VkEngine::getInstance()->getDevice(), 
		VkEngine::getInstance()->getCommandPool(), 
		VkEngine::getInstance()->getGraphicsQueue(), 
		image, 
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void Texture::initImageView()
{
	createImageView(VkEngine::getInstance()->getDevice(), image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, imageView);
}

void Texture::initSampler()
{
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.f;
	samplerInfo.minLod = 0.f;
	samplerInfo.maxLod = 0.f;

	VK_CHECK(vkCreateSampler(VkEngine::getInstance()->getDevice(), &samplerInfo, nullptr, &sampler));
}

void Texture::initDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = bindings.size();
	layoutInfo.pBindings = bindings.data();

	VK_CHECK(vkCreateDescriptorSetLayout(VkEngine::getInstance()->getDevice(), &layoutInfo, nullptr, &descriptorSetLayout));
}