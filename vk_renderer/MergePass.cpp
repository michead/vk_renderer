#include "MergePass.h"

#include "VkPool.h"


void MergePass::initAttachments()
{
	std::vector<VkAttachmentReference> attachmentReferences;

	colorAttachment = {};
	colorAttachment.format = VkEngine::getEngine().getSwapchainFormat();
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
#if SHOW_HUD
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
#else
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
#endif

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	attachmentReferences.push_back(colorAttachmentRef);

	VkSubpassDescription subPass = {};
	subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subPass.colorAttachmentCount = attachmentReferences.size();
	subPass.pColorAttachments = attachmentReferences.data();

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subPass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	renderPass = VkEngine::getEngine().getPool()->createRenderPass(renderPassInfo);
}

void MergePass::initCommandBuffers()
{
	commandBuffers.resize(VkEngine::getEngine().getSwapchainImageViews().size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = VkEngine::getEngine().getCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = commandBuffers.size();

	VK_CHECK(vkAllocateCommandBuffers(VkEngine::getEngine().getDevice(), &allocInfo, commandBuffers.data()));

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = TRANSPARENT_BLACK_CLEAR;
	clearValues[1].depthStencil = DEPTH_STENCIL_CLEAR;

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = VkEngine::getEngine().getSwapchainExtent();
	renderPassInfo.clearValueCount = clearValues.size();
	renderPassInfo.pClearValues = clearValues.data();

	std::vector<VkFramebuffer> framebuffers = VkEngine::getEngine().getSwapchainFramebuffers();

	for (size_t i = 0; i < commandBuffers.size(); i++)
	{
		vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

		renderPassInfo.framebuffer = framebuffers[i];

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		VkBuffer vertexBuffers[] = { quad->getVertexBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffers[i], quad->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(
			commandBuffers[i],
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			1,
			&descriptorSets[0],
			0,
			nullptr);

		vkCmdDrawIndexed(commandBuffers[i], quad->indices.size(), 1, 0, 0, 1);

		vkCmdEndRenderPass(commandBuffers[i]);

		VK_CHECK(vkEndCommandBuffer(commandBuffers[i]));
	}
}

void MergePass::initDescriptorSets()
{
	descriptorSets.resize(1);

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = VkEngine::getEngine().getDescriptorPool();
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout;

	VK_CHECK(vkAllocateDescriptorSets(VkEngine::getEngine().getDevice(), &allocInfo, &descriptorSets[0]));

	uint32_t bindingIndex = 0;

	std::vector<VkWriteDescriptorSet> descriptorWrites;

	VkDescriptorImageInfo colorImageInfo = {};
	colorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	colorImageInfo.imageView = diffuseAttachment->imageView;
	colorImageInfo.sampler = diffuseAttachment->imageSampler;

	VkWriteDescriptorSet colorDescriptorSet = {};
	colorDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	colorDescriptorSet.dstSet = descriptorSets[0];
	colorDescriptorSet.dstBinding = bindingIndex++;
	colorDescriptorSet.dstArrayElement = 0;
	colorDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	colorDescriptorSet.descriptorCount = 1;
	colorDescriptorSet.pImageInfo = &colorImageInfo;

	descriptorWrites.push_back(colorDescriptorSet);

	VkDescriptorImageInfo specularImageInfo = {};
	specularImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	specularImageInfo.imageView = specularAttachment->imageView;
	specularImageInfo.sampler = specularAttachment->imageSampler;

	VkWriteDescriptorSet specularDescriptorSet = {};
	specularDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	specularDescriptorSet.dstSet = descriptorSets[0];
	specularDescriptorSet.dstBinding = bindingIndex++;
	specularDescriptorSet.dstArrayElement = 0;
	specularDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	specularDescriptorSet.descriptorCount = 1;
	specularDescriptorSet.pImageInfo = &specularImageInfo;

	descriptorWrites.push_back(specularDescriptorSet);

	vkUpdateDescriptorSets(VkEngine::getEngine().getDevice(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

void MergePass::initDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	uint32_t bindingIndex = 0;

	VkDescriptorSetLayoutBinding diffuseSamplerLayoutBinding = {};
	diffuseSamplerLayoutBinding.binding = bindingIndex++;
	diffuseSamplerLayoutBinding.descriptorCount = 1;
	diffuseSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	diffuseSamplerLayoutBinding.pImmutableSamplers = nullptr;
	diffuseSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings.push_back(diffuseSamplerLayoutBinding);

	VkDescriptorSetLayoutBinding specularSamplerLayoutBinding = {};
	specularSamplerLayoutBinding.binding = bindingIndex++;
	specularSamplerLayoutBinding.descriptorCount = 1;
	specularSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	specularSamplerLayoutBinding.pImmutableSamplers = nullptr;
	specularSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings.push_back(specularSamplerLayoutBinding);

	descriptorSetLayout = VkEngine::getEngine().getPool()->createDescriptorSetLayout(bindings);
}

void MergePass::initGraphicsPipeline()
{
	std::vector<char> vs = readFile(vsPath);
	std::vector<char> fs = readFile(fsPath);
	std::vector<char> gs;

	if (!gsPath.empty())
	{
		gs = readFile(gsPath);
	}

	PipelineData pipelineData = VkEngine::getEngine().getPool()->createPipeline(
		renderPass,
		descriptorSetLayout,
		VkEngine::getEngine().getSwapchainExtent(),
		vs,
		fs,
		gs,
		1);

	pipeline = pipelineData.pipeline;
	pipelineLayout = pipelineData.pipelineLayout;
}