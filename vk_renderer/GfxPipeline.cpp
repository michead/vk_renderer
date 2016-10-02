#include "GfxPipeline.h"

#include "Camera.h"
#include "ShadowPass.h"
#include "LightingPass.h"
#include "GeometryPass.h"
#include "Scene.h"
#include "VkPool.h"
#include "VkUtils.h"


void GfxPipeline::init()
{
	shadowPass = new ShadowPass(SHADER_PATH"shadow/vert.spv", SHADER_PATH"shadow/frag.spv");
	geometryPass = new GeometryPass(SHADER_PATH"geometry/vert.spv", SHADER_PATH"geometry/frag.spv");
	lightingPass = new LightingPass(SHADER_PATH"lighting/vert.spv", SHADER_PATH"lighting/frag.spv", geometryPass->getGBuffer(), shadowPass->getMaps());

	shadowPass->init();
	geometryPass->init();
	lightingPass->init();

	shadowPassCompleteSemaphore = VkEngine::getEngine().getPool()->createSemaphore();
	geomPassCompleteSemaphore = VkEngine::getEngine().getPool()->createSemaphore();
	finalPassCompleteSemaphore = VkEngine::getEngine().getPool()->createSemaphore();
}

void GfxPipeline::run()
{
	uint32_t swapchainImgIndex = VkEngine::getEngine().getSwapchainImageIndex();

	VkSemaphore imageAvailableSemaphore = VkEngine::getEngine().getImageAvailableSemaphore();
	VkSemaphore renderCompleteSemaphore = VkEngine::getEngine().getRenderCompleteSemaphore();
	
	VkCommandBuffer geomPassCmdBuffer = geometryPass->getCurrentCmdBuffer();
	VkCommandBuffer finalPassCmdBuffer = lightingPass->getCurrentCmdBuffer();

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.signalSemaphoreCount = 1;

	for (size_t i = 0; i < shadowPass->getNumLights(); i++)
	{
		VkCommandBuffer cmdBuffer = shadowPass->getCmdBufferAt(i);
		VkSemaphore waitSemaphore, signalSemaphore;

		if (i == 0) waitSemaphore = imageAvailableSemaphore;
		else waitSemaphore = shadowPass->getSemaphoreAt(i - 1);

		if (i == shadowPass->getNumLights() - 1) signalSemaphore = shadowPassCompleteSemaphore;
		else signalSemaphore = shadowPass->getSemaphoreAt(i);

		submitInfo.pWaitSemaphores = &waitSemaphore;
		submitInfo.pSignalSemaphores = &signalSemaphore;
		submitInfo.pCommandBuffers = &cmdBuffer;

		VK_CHECK(vkQueueSubmit(VkEngine::getEngine().getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
	}

	submitInfo.pWaitSemaphores = &shadowPassCompleteSemaphore;
	submitInfo.pCommandBuffers = &geomPassCmdBuffer;
	submitInfo.pSignalSemaphores = &geomPassCompleteSemaphore;

	VK_CHECK(vkQueueSubmit(VkEngine::getEngine().getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));

	submitInfo.pWaitSemaphores = &geomPassCompleteSemaphore;
	submitInfo.pSignalSemaphores = &renderCompleteSemaphore;
	submitInfo.pCommandBuffers = &finalPassCmdBuffer;

	VK_CHECK(vkQueueSubmit(VkEngine::getEngine().getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
}

void GfxPipeline::initBufferData()
{
	shadowPass->initBufferData();
	geometryPass->initBufferData();
	lightingPass->initBufferData();
}

void GfxPipeline::updateBufferData()
{
	shadowPass->updateBufferData();
	geometryPass->updateBufferData();
	lightingPass->updateBufferData();
}

void GfxPipeline::cleanup()
{
	delete lightingPass;
	delete geometryPass;
}