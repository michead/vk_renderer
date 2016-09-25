#include "GfxPipeline.h"

#include "Camera.h"
#include "LightingPass.h"
#include "GeometryPass.h"
#include "Scene.h"
#include "VkPool.h"
#include "VkUtils.h"


void GfxPipeline::init()
{
	geometryPass = new GeometryPass(SHADER_PATH"geometry/vert.spv", SHADER_PATH"geometry/frag.spv");
	lightingPass = new LightingPass(SHADER_PATH"lighting/vert.spv", SHADER_PATH"lighting/frag.spv", geometryPass->getGBuffer());

	geometryPass->init();
	lightingPass->init();

	geomPassCompleteSemaphore = VkEngine::getEngine().getPool()->createSemaphore();
	finalPassCompleteSemaphore = VkEngine::getEngine().getPool()->createSemaphore();
}

void GfxPipeline::run()
{
	uint32_t swapchainImgIndex = VkEngine::getEngine().getSwapchainImageIndex();

	VkSemaphore imageAvailableSemaphore = VkEngine::getEngine().getImageAvailableSemaphore();
	VkSemaphore renderCompleteSemaphore = VkEngine::getEngine().getRenderCompleteSemaphore();
	
	VkCommandBuffer geomPassCmdBuffer = geometryPass->getCurrentCommandBuffer();
	VkCommandBuffer finalPassCmdBuffer = lightingPass->getCurrentCommandBuffer();

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &geomPassCmdBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &geomPassCompleteSemaphore;

	VK_CHECK(vkQueueSubmit(VkEngine::getEngine().getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));

	submitInfo.pWaitSemaphores = &geomPassCompleteSemaphore;
	submitInfo.pSignalSemaphores = &renderCompleteSemaphore;
	submitInfo.pCommandBuffers = &finalPassCmdBuffer;

	VK_CHECK(vkQueueSubmit(VkEngine::getEngine().getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
}

void GfxPipeline::initBufferData()
{
	lightingPass->initBufferData();
}

void GfxPipeline::updateBufferData()
{
	geometryPass->updateBufferData();
	lightingPass->updateBufferData();
}

void GfxPipeline::cleanup()
{
	delete lightingPass;
	delete geometryPass;
}