#include "ShadowPass.h"

#include "Camera.h"
#include "VkPool.h"


void ShadowPass::initAttachments()
{
	commandBuffers.resize(lights.size());

	VkAttachmentDescription attachmentDesc = {};
	attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	attachmentDesc.format = findDepthFormat(VkEngine::getEngine().getPhysicalDevice());

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 0;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.pColorAttachments = nullptr;
	subpass.colorAttachmentCount = 0;
	subpass.pDepthStencilAttachment = &depthReference;

	std::array<VkSubpassDependency, 2> dependencies;
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pAttachments = &attachmentDesc;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = dependencies.size();
	renderPassInfo.pDependencies = dependencies.data();

	renderPass = VkEngine::getEngine().getPool()->createRenderPass(renderPassInfo);

	for (size_t i = 0; i < lights.size(); i++)
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = VkEngine::getEngine().getCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		VK_CHECK(vkAllocateCommandBuffers(VkEngine::getEngine().getDevice(), &allocInfo, &commandBuffers[i]));

		attachments[i] = VkEngine::getEngine().getPool()->createGBufferAttachment(GBufferAttachmentType::DEPTH);

		VkExtent2D extent = VkEngine::getEngine().getSwapchainExtent();

		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.pNext = NULL;
		framebufferCreateInfo.renderPass = renderPass;
		framebufferCreateInfo.pAttachments = &attachments[i].imageView;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.width = extent.width;
		framebufferCreateInfo.height = extent.height;
		framebufferCreateInfo.layers = 1;

		framebuffers.push_back(VkEngine::getEngine().getPool()->createFramebuffer(framebufferCreateInfo));
	}
}

void ShadowPass::initSemaphores()
{
	for (size_t i = 0; i < lights.size() - 1; i++)
	{
		semaphores.push_back(VkEngine::getEngine().getPool()->createSemaphore());
	}
}

void ShadowPass::initCommandBuffers()
{
	static bool firstTime = true;
	if (!firstTime)
	{
		vkFreeCommandBuffers(
			VkEngine::getEngine().getDevice(),
			VkEngine::getEngine().getCommandPool(),
			lights.size(),
			commandBuffers.data());
	}
	else
	{
		firstTime = false;
	}

	for (size_t i = 0; i < lights.size(); i++)
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = VkEngine::getEngine().getCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		VK_CHECK(vkAllocateCommandBuffers(VkEngine::getEngine().getDevice(), &allocInfo, &commandBuffers[i]));

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

		VkExtent2D extent = VkEngine::getEngine().getSwapchainExtent();

		VkRect2D renderArea = {};
		renderArea.extent = extent;
		renderArea.offset = { 0, 0 };

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = framebuffers[i];
		renderPassInfo.renderArea = renderArea;

		VkClearValue clearValue = {};
		clearValue.depthStencil = DEPTH_STENCIL_CLEAR;

		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearValue;

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = {};
		viewport.width = (float) extent.width;
		viewport.height = (float) extent.height;
		viewport.minDepth = 0;
		viewport.maxDepth = 1;

		vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);
		vkCmdSetScissor(commandBuffers[i], 0, 1, &renderArea);
		vkCmdSetDepthBias(commandBuffers[i], DEPTH_BIAS_CONSTANT, 0.f, DEPTH_BIAS_SLOPE);
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		for (const auto& mesh : VkEngine::getEngine().getScene()->getMeshes())
		{
			loadMeshUniforms(mesh);

			VkBuffer vertexBuffers[] = { mesh->getVertexBuffer() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commandBuffers[i], mesh->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdBindDescriptorSets(
				commandBuffers[i],
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipelineLayout,
				0,
				1,
				&descriptorSets[i],
				0,
				nullptr);

			vkCmdDrawIndexed(commandBuffers[i], mesh->indices.size(), 1, 0, 0, 0);
		}

		vkCmdEndRenderPass(commandBuffers[i]);

		VK_CHECK(vkEndCommandBuffer(commandBuffers[i]));
	}
}

void ShadowPass::initDescriptorSets()
{
	descriptorSets.resize(lights.size());

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = VkEngine::getEngine().getDescriptorPool();
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout;

	for (size_t i = 0; i < lights.size(); i++)
	{
		VK_CHECK(vkAllocateDescriptorSets(VkEngine::getEngine().getDevice(), &allocInfo, &descriptorSets[i]));

		std::vector<VkWriteDescriptorSet> descriptorWrites;

		VkDescriptorBufferInfo cameraBufferInfo = {};
		cameraBufferInfo.buffer = cameraUniformBuffers[i];
		cameraBufferInfo.offset = 0;
		cameraBufferInfo.range = sizeof(CameraUniformBufferObject);

		VkWriteDescriptorSet cameraDescriptorSet = {};
		cameraDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		cameraDescriptorSet.dstSet = descriptorSets[i];
		cameraDescriptorSet.dstBinding = 0;
		cameraDescriptorSet.dstArrayElement = 0;
		cameraDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		cameraDescriptorSet.descriptorCount = 1;
		cameraDescriptorSet.pBufferInfo = &cameraBufferInfo;

		descriptorWrites.push_back(cameraDescriptorSet);

		VkDescriptorBufferInfo meshBufferInfo = {};
		meshBufferInfo.buffer = meshUniformBuffer;
		meshBufferInfo.offset = 0;
		meshBufferInfo.range = sizeof(MeshUniformBufferObject);

		VkWriteDescriptorSet meshDescriptorSet = {};
		meshDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		meshDescriptorSet.dstSet = descriptorSets[i];
		meshDescriptorSet.dstBinding = 1;
		meshDescriptorSet.dstArrayElement = 0;
		meshDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		meshDescriptorSet.descriptorCount = 1;
		meshDescriptorSet.pBufferInfo = &meshBufferInfo;

		descriptorWrites.push_back(meshDescriptorSet);
		
		vkUpdateDescriptorSets(VkEngine::getEngine().getDevice(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
	}
}

void ShadowPass::initDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings(2);

	VkDescriptorSetLayoutBinding cameraUBOLayoutBinding = {};
	cameraUBOLayoutBinding.binding = 0;
	cameraUBOLayoutBinding.descriptorCount = 1;
	cameraUBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	cameraUBOLayoutBinding.pImmutableSamplers = nullptr;
	cameraUBOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	bindings[0] = cameraUBOLayoutBinding;

	VkDescriptorSetLayoutBinding meshUBOLayoutBinding = {};
	meshUBOLayoutBinding.binding = 1;
	meshUBOLayoutBinding.descriptorCount = 1;
	meshUBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	meshUBOLayoutBinding.pImmutableSamplers = nullptr;
	meshUBOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	bindings[1] = meshUBOLayoutBinding;

	descriptorSetLayout = VkEngine::getEngine().getPool()->createDescriptorSetLayout(bindings);
}

void ShadowPass::initGraphicsPipeline()
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
		0);

	pipeline = pipelineData.pipeline;
	pipelineLayout = pipelineData.pipelineLayout;
}

void ShadowPass::initUniformBuffer()
{
	for (size_t i = 0; i < lights.size(); i++)
	{
		VkDeviceSize cameraBufferSize = sizeof(CameraUniformBufferObject);

		std::vector<BufferData> cameraBufferDataVec = VkEngine::getEngine().getPool()->createUniformBuffer(cameraBufferSize, true);
		cameraUniformStagingBuffers.push_back(cameraBufferDataVec[0].buffer);
		cameraUniformStagingBufferMemoryList.push_back(cameraBufferDataVec[0].bufferMemory);
		cameraUniformBuffers.push_back(cameraBufferDataVec[1].buffer);
		cameraUniformBufferMemoryList.push_back(cameraBufferDataVec[1].bufferMemory);
	}

	VkDeviceSize meshBufferSize = sizeof(MeshUniformBufferObject);

	std::vector<BufferData> meshBufferDataVec = VkEngine::getEngine().getPool()->createUniformBuffer(meshBufferSize, true);
	meshUniformStagingBuffer = meshBufferDataVec[0].buffer;
	meshUniformStagingBufferMemory = meshBufferDataVec[0].bufferMemory;
	meshUniformBuffer = meshBufferDataVec[1].buffer;
	meshUniformBufferMemory = meshBufferDataVec[1].bufferMemory;
}

void ShadowPass::loadMeshUniforms(const Mesh* mesh)
{
	MeshUniformBufferObject ubo = {};
	ubo.model = mesh->getModelMatrix();

	updateBuffer(
		VkEngine::getEngine().getDevice(),
		VkEngine::getEngine().getCommandPool(),
		VkEngine::getEngine().getGraphicsQueue(),
		&ubo,
		sizeof(ubo),
		meshUniformStagingBufferMemory,
		meshUniformBuffer,
		meshUniformStagingBuffer);
}

void ShadowPass::loadLightUniforms(size_t lightIndex)
{
	Camera* camera = VkEngine::getEngine().getScene()->getCamera();

	CameraUniformBufferObject ubo = {};
	ubo.view = lights[lightIndex]->getViewMatrix(camera->target);
	ubo.proj = camera->getProjMatrix();

	updateBuffer(
		VkEngine::getEngine().getDevice(),
		VkEngine::getEngine().getCommandPool(),
		VkEngine::getEngine().getGraphicsQueue(),
		&ubo,
		sizeof(ubo),
		cameraUniformStagingBufferMemoryList[lightIndex],
		cameraUniformBuffers[lightIndex],
		cameraUniformStagingBuffers[lightIndex]);
}

void ShadowPass::updateBufferData()
{
	for (size_t i = 0; i < lights.size(); i++)
	{
		loadLightUniforms(i);
	}
}