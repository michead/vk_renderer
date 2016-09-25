#pragma once

#include "VkUtils.h"


enum GBufferAttachmentType {
	COLOR,
	POSITION,
	NORMAL,
	DEPTH,
	NUM_TYPES
};


struct GBufferAttachment {
	VkImage image;
	VkImageView imageView;
	VkDeviceMemory imageMemory;
	VkSampler imageSampler;
};


struct GBuffer {
	void init();
	void bind();

	VkCommandBuffer commandBuffer;
	VkFramebuffer framebuffer;
	VkRenderPass renderPass;

	GBufferAttachment colorAttachment;
	GBufferAttachment positionAttachment;
	GBufferAttachment normalAttachment;
	GBufferAttachment depthAttachment;
};