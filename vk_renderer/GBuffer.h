#pragma once

#include <array>

#include "VkUtils.h"


#define GBUFFER_COLOR_ATTACH_ID		0
#define GBUFFER_POSITION_ATTACH_ID	1
#define GBUFFER_NORMAL_ATTACH_ID	2
#define GBUFFER_TANGENT_ATTACH_ID	3
#define GBUFFER_SPECULAR_ATTACH_ID	4
#define GBUFFER_MATERIAL_ATTACH_ID	5
#define GBUFFER_DEPTH_ATTACH_ID		6
#define GBUFFER_NUM_ATTACHMENTS		7


enum GBufferAttachmentType {
	COLOR,
	POSITION,
	NORMAL,
	TANGENT,
	SPECULAR,
	MATERIAL,
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