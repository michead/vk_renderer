#include "GfxPipeline.h"

#include "GeomPass.h"
#include "FinalPass.h"
#include "VkPool.h"


void GfxPipeline::init()
{
	geomPass = new GeomPass(SHADER_PATH"geom/vert.spv", SHADER_PATH"geom/frag.spv");
	finalPass = new FinalPass(SHADER_PATH"final/vert.spv", SHADER_PATH"final/frag.spv");

	geomPass->init();
	finalPass->init();

	geomPassCompleteSemaphore = VkEngine::getEngine().getPool()->createSemaphore();
	finalPassCompleteSemaphore = VkEngine::getEngine().getPool()->createSemaphore();
}

bool GfxPipeline::run()
{
	VkSemaphore imgAvailableSemaphore = VkEngine::getEngine().getImageAvailableSemaphore();
	uint32_t swapchainImgIndex = VkEngine::getEngine().getSwapchainImageIndex();
	
	VkCommandBuffer geomPassCmdBuffer = geomPass->getCurrentCommandBuffer();
	VkCommandBuffer finalPassCmdBuffer = finalPass->getCurrentCommandBuffer();

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imgAvailableSemaphore;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &geomPassCmdBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &geomPassCompleteSemaphore;

	VK_CHECK(vkQueueSubmit(VkEngine::getEngine().getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));

	submitInfo.pWaitSemaphores = &geomPassCompleteSemaphore;
	submitInfo.pSignalSemaphores = &finalPassCompleteSemaphore;
	submitInfo.pCommandBuffers = &finalPassCmdBuffer;

	VK_CHECK(vkQueueSubmit(VkEngine::getEngine().getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &finalPassCompleteSemaphore;

	VkSwapchainKHR swapChains[] = { VkEngine::getEngine().getSwapchain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &swapchainImgIndex;

	presentInfo.pResults = nullptr;

	return vkQueuePresentKHR(VkEngine::getEngine().getPresentationQueue(), &presentInfo) == VK_SUCCESS;
}

void GfxPipeline::updateData()
{

}

void GfxPipeline::cleanup()
{
	delete finalPass;
	delete geomPass;
}