#include "GfxPipeline.h"

#include "Camera.h"
#include "ShadowPass.h"
#include "LightingPass.h"
#include "GeometryPass.h"
#include "SSAOPass.h"
#include "SubsurfPass.h"
#include "MergePass.h"
#include "Scene.h"
#include "VkPool.h"
#include "VkUtils.h"


void GfxPipeline::init()
{
	shadowPass = new ShadowPass(SHADOW_PASS_VS, SHADOW_PASS_FS);
	geometryPass = new GeometryPass(GEOMETRY_PASS_VS, GEOMETRY_PASS_FS);
	ssaoPass = new SSAOPass(SSAO_MAIN_PASS_VS, SSAO_MAIN_PASS_FS, SSAO_BLUR_PASS_VS, SSAO_BLUR_PASS_FS, geometryPass->getGBuffer());
	lightingPass = new LightingPass(LIGHTING_PASS_VS, LIGHTING_PASS_FS, geometryPass->getGBuffer(), shadowPass->getNumLights(), 
		shadowPass->getMaps(), ssaoPass->getAOMap(), true);
	sssBlurPassOne = new SubsurfPass(SUBSURF_PASS_VS, SUBSURF_PASS_FS,
		glm::vec2(1, 0), geometryPass->getGBuffer(), lightingPass->getDiffuseAttachment());
	sssBlurPassTwo = new SubsurfPass(SUBSURF_PASS_VS, SUBSURF_PASS_FS,
		glm::vec2(0, 1), geometryPass->getGBuffer(), sssBlurPassOne->getColorAttachment());
	mergePass = new MergePass(MERGE_PASS_VS, MERGE_PASS_FS, sssBlurPassTwo->getColorAttachment(), lightingPass->getSpecularAttachment());

	shadowPass->init();
	geometryPass->init();
	ssaoPass->init();
	lightingPass->init();
	sssBlurPassOne->init();
	sssBlurPassTwo->init();
	mergePass->init();

	shadowPassCompleteSemaphore = VkEngine::getEngine().getPool()->createSemaphore();
	geomPassCompleteSemaphore = VkEngine::getEngine().getPool()->createSemaphore();
	mainSSAOPassCompleteSemaphore = VkEngine::getEngine().getPool()->createSemaphore();
	blurSSAOPassCompleteSemaphore = VkEngine::getEngine().getPool()->createSemaphore();
	mergePassCompleteSemaphore = VkEngine::getEngine().getPool()->createSemaphore();
	sssBlurPassOneCompleteSemaphore = VkEngine::getEngine().getPool()->createSemaphore();
	sssBlurPassTwoCompleteSemaphore = VkEngine::getEngine().getPool()->createSemaphore();
	lightingPassCompleteSemaphore = VkEngine::getEngine().getPool()->createSemaphore();
}

void GfxPipeline::run()
{
	uint32_t swapchainImgIndex = VkEngine::getEngine().getSwapchainImageIndex();

	VkSemaphore imageAvailableSemaphore = VkEngine::getEngine().getImageAvailableSemaphore();
	VkSemaphore renderingCompleteSemaphore = VkEngine::getEngine().getRenderCompleteSemaphore();
	
	VkCommandBuffer geomPassCmdBuffer = geometryPass->getCurrentCmdBuffer();
	VkCommandBuffer mainSSAOPassCmdBuffer = ssaoPass->getMainPassCmdBuffer();
	VkCommandBuffer blurSSAOPassCmdBuffer = ssaoPass->getBlurPassCmdBuffer();
	VkCommandBuffer lightingPassCmdBuffer = lightingPass->getCurrentCmdBuffer();
	VkCommandBuffer sssBlurPassOneCmdBuffer = sssBlurPassOne->getCurrentCmdBuffer();
	VkCommandBuffer sssBlurPassTwoCmdBuffer = sssBlurPassTwo->getCurrentCmdBuffer();
	VkCommandBuffer mergePassCmdBuffer = mergePass->getCurrentCmdBuffer();

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
	submitInfo.pCommandBuffers = &mainSSAOPassCmdBuffer;
	submitInfo.pSignalSemaphores = &mainSSAOPassCompleteSemaphore;

	VK_CHECK(vkQueueSubmit(VkEngine::getEngine().getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));

	submitInfo.pWaitSemaphores = &mainSSAOPassCompleteSemaphore;
	submitInfo.pCommandBuffers = &blurSSAOPassCmdBuffer;
	submitInfo.pSignalSemaphores = &blurSSAOPassCompleteSemaphore;

	VK_CHECK(vkQueueSubmit(VkEngine::getEngine().getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));

	submitInfo.pWaitSemaphores = &blurSSAOPassCompleteSemaphore;
	submitInfo.pSignalSemaphores = &lightingPassCompleteSemaphore;
	submitInfo.pCommandBuffers = &lightingPassCmdBuffer;

	VK_CHECK(vkQueueSubmit(VkEngine::getEngine().getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));

	submitInfo.pWaitSemaphores = &lightingPassCompleteSemaphore;
	submitInfo.pSignalSemaphores = &sssBlurPassOneCompleteSemaphore;
	submitInfo.pCommandBuffers = &sssBlurPassOneCmdBuffer;

	VK_CHECK(vkQueueSubmit(VkEngine::getEngine().getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));

	submitInfo.pWaitSemaphores = &sssBlurPassOneCompleteSemaphore;
	submitInfo.pSignalSemaphores = &sssBlurPassTwoCompleteSemaphore;
	submitInfo.pCommandBuffers = &sssBlurPassTwoCmdBuffer;

	VK_CHECK(vkQueueSubmit(VkEngine::getEngine().getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));

	submitInfo.pWaitSemaphores = &sssBlurPassTwoCompleteSemaphore;
	submitInfo.pSignalSemaphores = &renderingCompleteSemaphore;
	submitInfo.pCommandBuffers = &mergePassCmdBuffer;

	VK_CHECK(vkQueueSubmit(VkEngine::getEngine().getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
}

VkCommandBuffer GfxPipeline::getPresentationCmdBuffer() const
{ 
	return mergePass->getCurrentCmdBuffer(); 
}

void GfxPipeline::initBufferData()
{
	shadowPass->initBufferData();
	geometryPass->initBufferData();
	ssaoPass->initBufferData();
	lightingPass->initBufferData();
	sssBlurPassOne->initBufferData();
	sssBlurPassTwo->initBufferData();
	mergePass->initBufferData();
}

void GfxPipeline::updateBufferData()
{
	shadowPass->updateBufferData();
	geometryPass->updateBufferData();
	ssaoPass->updateBufferData();
	lightingPass->updateBufferData();
	sssBlurPassOne->updateBufferData();
	sssBlurPassTwo->updateBufferData();
	mergePass->updateBufferData();
}

void GfxPipeline::cleanup()
{
	delete lightingPass;
	delete geometryPass;
	delete shadowPass;
	delete ssaoPass;
	delete sssBlurPassTwo;
	delete sssBlurPassOne;
	delete mergePass;
}