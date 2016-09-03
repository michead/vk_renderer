#pragma once

#include "VkApp.h"
#include "VkObjWrapper.h"

enum GBufferAttachmentType {
	GBUFFER_DIFFUSE,
	GBUFFER_TEXCOORD,
	GBUFFER_NORMAL,
	GBUFFER_DEPTH,
	GBUFFER_POSITION,
	GBUFFER_NUM_TEXTURES
};

class GBuffer {
public:
	void init();
	void bind();

private:
	VK_VEC_WRAP(VkImage) textureImages;
	VK_VEC_WRAP(VkImageView) textureImageViews;
	VK_VEC_WRAP(VkSampler) textureSamplers;
	VK_VEC_WRAP(VkDeviceMemory) textureImageMemories;
	VK_WRAP(VkImage) depthImage { VkApp::getDevice(), vkDestroyImage };
	VK_WRAP(VkDeviceMemory) depthImageMemory { VkApp::getDevice(), vkFreeMemory };
	VK_WRAP(VkImageView) depthImageView { VkApp::getDevice(), vkDestroyImageView };
};