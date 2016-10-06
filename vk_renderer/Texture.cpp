#pragma once

#include "Texture.h"

#include <array>
#include <string>

#include "VkUtils.h"
#include "VkPool.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm\glm.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb\stb_image.h>


void Texture::init()
{
	int texChannels;
	pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	if (!pixels)
	{
		throw std::runtime_error("Failed to load texture image!");
	}

	initResources();
}

void Texture::initResources()
{
	ImageData imageData = VkEngine::getEngine().getPool()->createTextureResources(pixels, texWidth, texHeight, highPrec);
	image = imageData.image;
	imageView = imageData.imageView;
	imageMemory = imageData.imageMemory;
	sampler = imageData.sampler;

	stbi_image_free(pixels);
}