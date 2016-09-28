#pragma once

#include <array>

#include "VkUtils.h"


#define GBUFFER_COLOR_ATTACH_ID		0
#define GBUFFER_POSITION_ATTACH_ID	1
#define GBUFFER_NORMAL_ATTACH_ID	2
#define GBUFFER_DEPTH_ATTACH_ID		3
#define GBUFFER_NUM_ATTACHMENTS		4


enum GBufferAttachmentType {
	COLOR,
	POSITION,
	NORMAL,
	DEPTH,
	NUM_TYPES
};


struct GBufferAttachment {
	GBufferAttachmentType type;
	VkImage image;
	VkImageView imageView;
	VkDeviceMemory imageMemory;
	VkSampler imageSampler;
};


struct GBuffer {
	void init();

	VkCommandBuffer commandBuffer;
	VkFramebuffer framebuffer;
	VkRenderPass renderPass;

	std::array<GBufferAttachment, GBUFFER_NUM_ATTACHMENTS> attachments;
};