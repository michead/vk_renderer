#include "GeometryPass.h"

#include "Camera.h"
#include "Scene.h"
#include "VkPool.h"


void GeometryPass::initAttachments()
{
	gBuffer.init();
}

void GeometryPass::initCommandBuffers()
{
	static bool firstTime = true;
	if (!firstTime)
	{
		vkFreeCommandBuffers(
			VkEngine::getEngine().getDevice(),
			VkEngine::getEngine().getCommandPool(),
			1,
			&commandBuffer);
	}
	else
	{
		firstTime = false;
	}

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

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkExtent2D extent = VkEngine::getEngine().getSwapchainExtent();

	VkRect2D renderArea = {};
	renderArea.extent = extent;
	renderArea.offset = { 0, 0 };

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = gBuffer.renderPass;
	renderPassInfo.framebuffer = gBuffer.framebuffer;
	renderPassInfo.renderArea = renderArea;

	std::array<VkClearValue, GBUFFER_NUM_ATTACHMENTS> clearValues = {};
	clearValues[0].color = TRANSPARENT_BLACK_CLEAR;
	clearValues[1].color = TRANSPARENT_BLACK_CLEAR;
	clearValues[2].color = TRANSPARENT_BLACK_CLEAR;
	clearValues[3].color = TRANSPARENT_BLACK_CLEAR;
	clearValues[4].color = TRANSPARENT_BLACK_CLEAR;
	clearValues[5].color = TRANSPARENT_BLACK_CLEAR;
	clearValues[6].depthStencil = DEPTH_STENCIL_CLEAR;

	renderPassInfo.clearValueCount = clearValues.size();
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport = {};
	viewport.width = (float)extent.width;
	viewport.height = (float)extent.height;
	viewport.minDepth = 0;
	viewport.maxDepth = 1;

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &renderArea);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	for (const auto& mesh : VkEngine::getEngine().getScene()->getMeshes())
	{
		if (loadedMaterial != mesh->material->id)
		{ 
			loadMaterial(mesh->material);
			loadedMaterial = mesh->material->id;
		}

		loadMeshUniforms(mesh);

		VkBuffer vertexBuffers[] = { mesh->getVertexBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, mesh->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			1,
			&descriptorSets[mesh->material->id],
			0,
			nullptr);
		
		vkCmdDrawIndexed(commandBuffer, mesh->indices.size(), 1, 0, 0, 0);
	}

	vkCmdEndRenderPass(commandBuffer);

	VK_CHECK(vkEndCommandBuffer(commandBuffer));
}

void GeometryPass::loadMaterial(const Material* material)
{
	GPMaterialUniformBufferObject ubo = {};
	ubo.kd = glm::vec4(material->kd, 1);
	ubo.ks = glm::vec4(material->ks, 1);
	ubo.rs = material->rs;
	ubo.opacity = material->opacity;
	ubo.translucency = material->translucency;
	ubo.subsurfWidth = material->subsurfWidth;

	updateBuffer(
		VkEngine::getEngine().getDevice(),
		VkEngine::getEngine().getCommandPool(),
		VkEngine::getEngine().getGraphicsQueue(),
		&ubo,
		sizeof(ubo),
		materialUniformStagingBufferMemory,
		materialUniformBuffer,
		materialUniformStagingBuffer);
}

void GeometryPass::loadMeshUniforms(const Mesh* mesh)
{
	GPMeshUniformBufferObject ubo = {};
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

void GeometryPass::initDescriptorSets()
{
	std::vector<Material*> materials = VkEngine::getEngine().getScene()->getMaterials();
	descriptorSets.resize(materials.size());

	int16_t m = 0;
	for (const auto& material : materials)
	{
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = VkEngine::getEngine().getDescriptorPool();
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorSetLayout;

		VK_CHECK(vkAllocateDescriptorSets(VkEngine::getEngine().getDevice(), &allocInfo, &descriptorSets[m]));

		std::vector<VkDescriptorImageInfo> imageInfos;
	
		VkDescriptorImageInfo albedoInfo = {};
		albedoInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		albedoInfo.imageView = material->kdMap->getImageView();
		albedoInfo.sampler = material->kdMap->getSampler();

		imageInfos.push_back(albedoInfo);

		VkDescriptorImageInfo normalInfo = {};
		normalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		normalInfo.imageView = material->normalMap->getImageView();
		normalInfo.sampler = material->normalMap->getSampler();
			
		imageInfos.push_back(normalInfo);

		std::vector<VkWriteDescriptorSet> descriptorWrites;

		VkDescriptorBufferInfo cameraBufferInfo = {};
		cameraBufferInfo.buffer = cameraUniformBuffer;
		cameraBufferInfo.offset = 0;
		cameraBufferInfo.range = sizeof(GPCameraUniformBufferObject);

		VkWriteDescriptorSet cameraDescriptorSet = {};
		cameraDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		cameraDescriptorSet.dstSet = descriptorSets[m];
		cameraDescriptorSet.dstBinding = 0;
		cameraDescriptorSet.dstArrayElement = 0;
		cameraDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		cameraDescriptorSet.descriptorCount = 1;
		cameraDescriptorSet.pBufferInfo = &cameraBufferInfo;

		descriptorWrites.push_back(cameraDescriptorSet);

		VkDescriptorBufferInfo meshBufferInfo = {};
		meshBufferInfo.buffer = meshUniformBuffer;
		meshBufferInfo.offset = 0;
		meshBufferInfo.range = sizeof(GPMeshUniformBufferObject);

		VkWriteDescriptorSet meshDescriptorSet = {};
		meshDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		meshDescriptorSet.dstSet = descriptorSets[m];
		meshDescriptorSet.dstBinding = 1;
		meshDescriptorSet.dstArrayElement = 0;
		meshDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		meshDescriptorSet.descriptorCount = 1;
		meshDescriptorSet.pBufferInfo = &meshBufferInfo;

		descriptorWrites.push_back(meshDescriptorSet);

		for (uint16_t i = 2; i <= imageInfos.size(); i++)
		{
			VkWriteDescriptorSet mapDescriptorSet = {};
			mapDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			mapDescriptorSet.dstSet = descriptorSets[m];
			mapDescriptorSet.dstBinding = i;
			mapDescriptorSet.dstArrayElement = 0;
			mapDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			mapDescriptorSet.descriptorCount = 1;
			mapDescriptorSet.pImageInfo = &imageInfos[i - 2];

			descriptorWrites.push_back(mapDescriptorSet);
		}

		VkDescriptorBufferInfo materialBufferInfo = {};
		materialBufferInfo.buffer = materialUniformBuffer;
		materialBufferInfo.offset = 0;
		materialBufferInfo.range = sizeof(GPMaterialUniformBufferObject);

		VkWriteDescriptorSet materialDescriptorSet = {};
		materialDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		materialDescriptorSet.dstSet = descriptorSets[m];
		materialDescriptorSet.dstBinding = 4;
		materialDescriptorSet.dstArrayElement = 0;
		materialDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		materialDescriptorSet.descriptorCount = 1;
		materialDescriptorSet.pBufferInfo = &materialBufferInfo;

		descriptorWrites.push_back(materialDescriptorSet);

		vkUpdateDescriptorSets(VkEngine::getEngine().getDevice(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

		m++;
	}
}

void GeometryPass::initBufferData()
{

}

void GeometryPass::updateBufferData()
{
	loadMaterial(VkEngine::getEngine().getScene()->getMaterials()[0]);

	GPCameraUniformBufferObject ubo = {};
	ubo.view = VkEngine::getEngine().getScene()->getCamera()->getViewMatrix();
	ubo.proj = VkEngine::getEngine().getScene()->getCamera()->getProjMatrix();

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

void GeometryPass::initGraphicsPipeline()
{
	std::vector<char> vs = readFile(vsPath);
	std::vector<char> fs = readFile(fsPath);
	std::vector<char> gs;

	if (!gsPath.empty())
	{
		gs = readFile(gsPath);
	}

	PipelineData pipelineData = VkEngine::getEngine().getPool()->createPipeline(
		gBuffer.renderPass,
		descriptorSetLayout,
		VkEngine::getEngine().getSwapchainExtent(),
		vs,
		fs,
		gs);

	pipeline = pipelineData.pipeline;
	pipelineLayout = pipelineData.pipelineLayout;
}

void GeometryPass::initUniformBuffer()
{
	VkDeviceSize cameraBufferSize = sizeof(GPCameraUniformBufferObject);

	std::vector<BufferData> cameraBufferDataVec = VkEngine::getEngine().getPool()->createUniformBuffer(cameraBufferSize, true);
	cameraUniformStagingBuffer = cameraBufferDataVec[0].buffer;
	cameraUniformStagingBufferMemory = cameraBufferDataVec[0].bufferMemory;
	cameraUniformBuffer = cameraBufferDataVec[1].buffer;
	cameraUniformBufferMemory = cameraBufferDataVec[1].bufferMemory;

	VkDeviceSize meshBufferSize = sizeof(GPMeshUniformBufferObject);

	std::vector<BufferData> meshBufferDataVec = VkEngine::getEngine().getPool()->createUniformBuffer(meshBufferSize, true);
	meshUniformStagingBuffer = meshBufferDataVec[0].buffer;
	meshUniformStagingBufferMemory = meshBufferDataVec[0].bufferMemory;
	meshUniformBuffer = meshBufferDataVec[1].buffer;
	meshUniformBufferMemory = meshBufferDataVec[1].bufferMemory;

	VkDeviceSize materialBufferSize = sizeof(GPMaterialUniformBufferObject);

	std::vector<BufferData> materialBufferDataVec = VkEngine::getEngine().getPool()->createUniformBuffer(materialBufferSize, true);
	materialUniformStagingBuffer = materialBufferDataVec[0].buffer;
	materialUniformStagingBufferMemory = materialBufferDataVec[0].bufferMemory;
	materialUniformBuffer = materialBufferDataVec[1].buffer;
	materialUniformBufferMemory = materialBufferDataVec[1].bufferMemory;
}

void GeometryPass::initDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings(5);

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
	meshUBOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings[1] = meshUBOLayoutBinding;

	VkDescriptorSetLayoutBinding samplerAlbedoLayoutBinding = {};
	samplerAlbedoLayoutBinding.binding = 2;
	samplerAlbedoLayoutBinding.descriptorCount = 1;
	samplerAlbedoLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerAlbedoLayoutBinding.pImmutableSamplers = nullptr;
	samplerAlbedoLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings[2] = samplerAlbedoLayoutBinding;

	VkDescriptorSetLayoutBinding samplerNormalLayoutBinding = {};
	samplerNormalLayoutBinding.binding = 3;
	samplerNormalLayoutBinding.descriptorCount = 1;
	samplerNormalLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerNormalLayoutBinding.pImmutableSamplers = nullptr;
	samplerNormalLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings[3] = samplerNormalLayoutBinding;

	VkDescriptorSetLayoutBinding materialUBOLayoutBinding = {};
	materialUBOLayoutBinding.binding = 4;
	materialUBOLayoutBinding.descriptorCount = 1;
	materialUBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	materialUBOLayoutBinding.pImmutableSamplers = nullptr;
	materialUBOLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings[4] = materialUBOLayoutBinding;

	descriptorSetLayout = VkEngine::getEngine().getPool()->createDescriptorSetLayout(bindings);
}