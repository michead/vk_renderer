#pragma once

#include "VkEngine.h"


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
	GBuffer() { }
	~GBuffer() { cleanup(); }

	void init();
	void bind();

private:
	std::vector<VkImage> textureImages;
	std::vector<VkImageView> textureImageViews;
	std::vector<VkSampler> textureSamplers;
	std::vector<VkDeviceMemory> textureImageMemories;
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	void cleanup();
};