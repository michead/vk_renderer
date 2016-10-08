#include "SubsurfPass.h"

#include "Camera.h"
#include "VkPool.h"


void SubsurfPass::initAttachments()
{
	VkAttachmentDescription attachmentDesc = {};
	attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	attachmentDesc.format = VK_FORMAT_R8G8B8A8_UNORM;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subPass = {};
	subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subPass.colorAttachmentCount = 1;
	subPass.pColorAttachments = &colorAttachmentRef;

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
	renderPassInfo.pAttachments = &attachmentDesc;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subPass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	renderPass = VkEngine::getEngine().getPool()->createRenderPass(renderPassInfo);

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = VkEngine::getEngine().getCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VK_CHECK(vkAllocateCommandBuffers(VkEngine::getEngine().getDevice(), &allocInfo, &commandBuffer));

	attachment = VkEngine::getEngine().getPool()->createGBufferAttachment(GBufferAttachmentType::COLOR);

	VkExtent2D extent = VkEngine::getEngine().getSwapchainExtent();

	VkFramebufferCreateInfo framebufferCreateInfo = {};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.pNext = NULL;
	framebufferCreateInfo.renderPass = renderPass;
	framebufferCreateInfo.pAttachments = &attachment.imageView;
	framebufferCreateInfo.attachmentCount = 1;
	framebufferCreateInfo.width = extent.width;
	framebufferCreateInfo.height = extent.height;
	framebufferCreateInfo.layers = 1;

	framebuffer = VkEngine::getEngine().getPool()->createFramebuffer(framebufferCreateInfo);
}

void SubsurfPass::initCommandBuffers()
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = VkEngine::getEngine().getCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VK_CHECK(vkAllocateCommandBuffers(VkEngine::getEngine().getDevice(), &allocInfo, &commandBuffer));

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

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	renderPassInfo.framebuffer = framebuffer;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	VkBuffer vertexBuffers[] = { quad->getVertexBuffer() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, quad->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipelineLayout,
		0,
		1,
		&descriptorSets[0],
		0,
		nullptr);

	vkCmdDrawIndexed(commandBuffer, quad->indices.size(), 1, 0, 0, 1);

	vkCmdEndRenderPass(commandBuffer);

	VK_CHECK(vkEndCommandBuffer(commandBuffer));
}

void SubsurfPass::initDescriptorSets()
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
	colorImageInfo.imageView = inColorAttachment->imageView;
	colorImageInfo.sampler = inColorAttachment->imageSampler;

	VkWriteDescriptorSet colorDescriptorSet = {};
	colorDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	colorDescriptorSet.dstSet = descriptorSets[0];
	colorDescriptorSet.dstBinding = bindingIndex++;
	colorDescriptorSet.dstArrayElement = 0;
	colorDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	colorDescriptorSet.descriptorCount = 1;
	colorDescriptorSet.pImageInfo = &colorImageInfo;

	descriptorWrites.push_back(colorDescriptorSet);

	VkDescriptorImageInfo depthImageInfo = {};
	depthImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	depthImageInfo.imageView = gBuffer->attachments[GBUFFER_DEPTH_ATTACH_ID].imageView;
	depthImageInfo.sampler = gBuffer->attachments[GBUFFER_DEPTH_ATTACH_ID].imageSampler;

	VkWriteDescriptorSet depthDescriptorSet = {};
	depthDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	depthDescriptorSet.dstSet = descriptorSets[0];
	depthDescriptorSet.dstBinding = bindingIndex++;
	depthDescriptorSet.dstArrayElement = 0;
	depthDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	depthDescriptorSet.descriptorCount = 1;
	depthDescriptorSet.pImageInfo = &depthImageInfo;

	descriptorWrites.push_back(depthDescriptorSet);

	VkDescriptorImageInfo materialImageInfo = {};
	materialImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	materialImageInfo.imageView = gBuffer->attachments[GBUFFER_MATERIAL_ATTACH_ID].imageView;
	materialImageInfo.sampler = gBuffer->attachments[GBUFFER_MATERIAL_ATTACH_ID].imageSampler;

	VkWriteDescriptorSet materialDescriptorSet = {};
	materialDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	materialDescriptorSet.dstSet = descriptorSets[0];
	materialDescriptorSet.dstBinding = bindingIndex++;
	materialDescriptorSet.dstArrayElement = 0;
	materialDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	materialDescriptorSet.descriptorCount = 1;
	materialDescriptorSet.pImageInfo = &materialImageInfo;

	descriptorWrites.push_back(materialDescriptorSet);

	VkDescriptorBufferInfo cameraBufferInfo = {};
	cameraBufferInfo.buffer = cameraUniformBuffer;
	cameraBufferInfo.offset = 0;
	cameraBufferInfo.range = sizeof(SSSPCameraUniformBufferObject);

	VkWriteDescriptorSet cameraDescriptorSet = {};
	cameraDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	cameraDescriptorSet.dstSet = descriptorSets[0];
	cameraDescriptorSet.dstBinding = bindingIndex++;
	cameraDescriptorSet.dstArrayElement = 0;
	cameraDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	cameraDescriptorSet.descriptorCount = 1;
	cameraDescriptorSet.pBufferInfo = &cameraBufferInfo;

	descriptorWrites.push_back(cameraDescriptorSet);

	VkDescriptorBufferInfo instanceBufferInfo = {};
	instanceBufferInfo.buffer = instanceUniformBuffer;
	instanceBufferInfo.offset = 0;
	instanceBufferInfo.range = sizeof(SSSPInstanceUniformBufferObject);

	VkWriteDescriptorSet instanceDescriptorSet = {};
	instanceDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	instanceDescriptorSet.dstSet = descriptorSets[0];
	instanceDescriptorSet.dstBinding = bindingIndex++;
	instanceDescriptorSet.dstArrayElement = 0;
	instanceDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	instanceDescriptorSet.descriptorCount = 1;
	instanceDescriptorSet.pBufferInfo = &instanceBufferInfo;

	descriptorWrites.push_back(instanceDescriptorSet);

	vkUpdateDescriptorSets(VkEngine::getEngine().getDevice(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

void SubsurfPass::initGraphicsPipeline()
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

void SubsurfPass::initDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	uint32_t bindingIndex = 0;

	VkDescriptorSetLayoutBinding colorSamplerLayoutBinding = {};
	colorSamplerLayoutBinding.binding = bindingIndex++;
	colorSamplerLayoutBinding.descriptorCount = 1;
	colorSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	colorSamplerLayoutBinding.pImmutableSamplers = nullptr;
	colorSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings.push_back(colorSamplerLayoutBinding);

	VkDescriptorSetLayoutBinding depthSamplerLayoutBinding = {};
	depthSamplerLayoutBinding.binding = bindingIndex++;
	depthSamplerLayoutBinding.descriptorCount = 1;
	depthSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	depthSamplerLayoutBinding.pImmutableSamplers = nullptr;
	depthSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings.push_back(depthSamplerLayoutBinding);

	VkDescriptorSetLayoutBinding materialSamplerLayoutBinding = {};
	materialSamplerLayoutBinding.binding = bindingIndex++;
	materialSamplerLayoutBinding.descriptorCount = 1;
	materialSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	materialSamplerLayoutBinding.pImmutableSamplers = nullptr;
	materialSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings.push_back(materialSamplerLayoutBinding);

	VkDescriptorSetLayoutBinding cameraLayoutBinding = {};
	cameraLayoutBinding.binding = bindingIndex++;
	cameraLayoutBinding.descriptorCount = 1;
	cameraLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	cameraLayoutBinding.pImmutableSamplers = nullptr;
	cameraLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings.push_back(cameraLayoutBinding);

	VkDescriptorSetLayoutBinding instanceLayoutBinding = {};
	instanceLayoutBinding.binding = bindingIndex++;
	instanceLayoutBinding.descriptorCount = 1;
	instanceLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	instanceLayoutBinding.pImmutableSamplers = nullptr;
	instanceLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings.push_back(instanceLayoutBinding);

	descriptorSetLayout = VkEngine::getEngine().getPool()->createDescriptorSetLayout(bindings);
}

void SubsurfPass::initUniformBuffer()
{
	VkDeviceSize cameraBufferSize = sizeof(SSSPCameraUniformBufferObject);

	std::vector<BufferData> cameraBufferDataVec = VkEngine::getEngine().getPool()->createUniformBuffer(cameraBufferSize, true);
	cameraUniformStagingBuffer = cameraBufferDataVec[0].buffer;
	cameraUniformStagingBufferMemory = cameraBufferDataVec[0].bufferMemory;
	cameraUniformBuffer = cameraBufferDataVec[1].buffer;
	cameraUniformBufferMemory = cameraBufferDataVec[1].bufferMemory;

	VkDeviceSize instanceBufferSize = sizeof(SSSPInstanceUniformBufferObject);

	std::vector<BufferData> instanceBufferDataVec = VkEngine::getEngine().getPool()->createUniformBuffer(instanceBufferSize, true);
	instanceUniformStagingBuffer = instanceBufferDataVec[0].buffer;
	instanceUniformStagingBufferMemory = instanceBufferDataVec[0].bufferMemory;
	instanceUniformBuffer = instanceBufferDataVec[1].buffer;
	instanceUniformBufferMemory = instanceBufferDataVec[1].bufferMemory;
}

void SubsurfPass::loadInstanceUniforms()
{
	SSSPInstanceUniformBufferObject ubo = {};
	ubo.blurDirection = blurDirection;
	for (size_t i = 0; i < SS_NUM_SAMPLES; i++) ubo.kernel[i] = kernel[i];

	updateBuffer(
		VkEngine::getEngine().getDevice(),
		VkEngine::getEngine().getCommandPool(),
		VkEngine::getEngine().getGraphicsQueue(),
		&ubo,
		sizeof(ubo),
		instanceUniformStagingBufferMemory,
		instanceUniformBuffer,
		instanceUniformStagingBuffer);
}

void SubsurfPass::loadCameraUniforms()
{
	Camera* camera = VkEngine::getEngine().getScene()->getCamera();

	SSSPCameraUniformBufferObject ubo = {};
	ubo.fovy = camera->fovy;

	updateBuffer(
		VkEngine::getEngine().getDevice(),
		VkEngine::getEngine().getCommandPool(),
		VkEngine::getEngine().getGraphicsQueue(),
		&ubo,
		sizeof(ubo),
		cameraUniformStagingBufferMemory,
		cameraUniformBuffer,
		cameraUniformStagingBuffer);
}

void SubsurfPass::initBufferData()
{
	loadInstanceUniforms();
}

void SubsurfPass::updateBufferData()
{
	loadCameraUniforms();
}

void SubsurfPass::computeKernel(glm::vec3 strength, glm::vec3 falloff)
{
	static const float range = 2;
	static const float exponent = 2;

	float step = 2 * range / (SS_NUM_SAMPLES - 1);

	for (int i = 0; i < SS_NUM_SAMPLES; i++)
	{
		float o = -range + float(i) * step;
		float sign = o < 0 ? -1.f : 1.f;
		kernel[i].w = range * sign * abs(pow(o, exponent)) / pow(range, exponent);
	}

	for (int i = 0; i < SS_NUM_SAMPLES; i++)
	{
		float w0 = i > 0 ? abs(kernel[i].w - kernel[i - 1].w) : 0;
		float w1 = i < SS_NUM_SAMPLES - 1 ? abs(kernel[i].w - kernel[i + 1].w) : 0;
		float area = (w0 + w1) / 2.f;
		glm::vec3 t = area * profile(falloff, kernel[i].w);
		kernel[i].x = t.x;
		kernel[i].y = t.y;
		kernel[i].z = t.z;
	}

	glm::vec4 t = kernel[SS_NUM_SAMPLES / 2];
	for (int i = SS_NUM_SAMPLES / 2; i > 0; i--)
		kernel[i] = kernel[i - 1];
	kernel[0] = t;

	glm::vec3 sum = glm::vec3(0);
	for (int i = 0; i < SS_NUM_SAMPLES; i++)
		sum += glm::vec3(kernel[i].x, kernel[i].y, kernel[i].z);

	for (int i = 0; i < SS_NUM_SAMPLES; i++)
	{
		kernel[i].x /= sum.x;
		kernel[i].y /= sum.y;
		kernel[i].z /= sum.z;
	}

	kernel[0].x = (1.f - strength.x) + strength.x * kernel[0].x;
	kernel[0].y = (1.f - strength.y) + strength.y * kernel[0].y;
	kernel[0].z = (1.f - strength.z) + strength.z * kernel[0].z;

	for (int i = 1; i < SS_NUM_SAMPLES; i++)
	{
		kernel[i].x *= strength.x;
		kernel[i].y *= strength.y;
		kernel[i].z *= strength.z;
	}
}