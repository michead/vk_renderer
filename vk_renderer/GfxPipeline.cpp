#include "GfxPipeline.h"

#include <chrono>

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

#ifdef PERF_GPU_TIME
		uint64_t shadowStart = 0;
		VK_CHECK(vkGetQueryPoolResults(VkEngine::getEngine().getDevice(), VkEngine::getEngine().getQueryPool(),
			2 * i, 1, sizeof(uint64_t),
			&shadowStart, sizeof(uint64_t),
			VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));

		uint64_t shadowEnd = 0;
		VK_CHECK(vkGetQueryPoolResults(VkEngine::getEngine().getDevice(), VkEngine::getEngine().getQueryPool(),
			2 * i + 1, 1, sizeof(uint64_t),
			&shadowEnd, sizeof(uint64_t),
			VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));

		timeSpentInShadowPass += (shadowEnd - shadowStart) / 1000.0;
#endif
	}

	submitInfo.pWaitSemaphores = &shadowPassCompleteSemaphore;
	submitInfo.pCommandBuffers = &geomPassCmdBuffer;
	submitInfo.pSignalSemaphores = &geomPassCompleteSemaphore;

	VK_CHECK(vkQueueSubmit(VkEngine::getEngine().getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
#ifdef PERF_GPU_TIME
	uint64_t geomStart = 0;
	VK_CHECK(vkGetQueryPoolResults(VkEngine::getEngine().getDevice(), VkEngine::getEngine().getQueryPool(),
		8, 1, sizeof(uint64_t),
		&geomStart, sizeof(uint64_t),
		VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));

	uint64_t geomEnd = 0;
	VK_CHECK(vkGetQueryPoolResults(VkEngine::getEngine().getDevice(), VkEngine::getEngine().getQueryPool(),
		9, 1, sizeof(uint64_t),
		&geomEnd, sizeof(uint64_t),
		VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));

	timeSpentInGeomPass += (geomEnd - geomStart) / 1000.0;
#endif

	submitInfo.pWaitSemaphores = &geomPassCompleteSemaphore;
	submitInfo.pCommandBuffers = &mainSSAOPassCmdBuffer;
	submitInfo.pSignalSemaphores = &mainSSAOPassCompleteSemaphore;

	VK_CHECK(vkQueueSubmit(VkEngine::getEngine().getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
#ifdef PERF_GPU_TIME
	uint64_t mainSSAOStart = 0;
	VK_CHECK(vkGetQueryPoolResults(VkEngine::getEngine().getDevice(), VkEngine::getEngine().getQueryPool(),
		10, 1, sizeof(uint64_t),
		&mainSSAOStart, sizeof(uint64_t),
		VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));

	uint64_t mainSSAOEnd = 0;
	VK_CHECK(vkGetQueryPoolResults(VkEngine::getEngine().getDevice(), VkEngine::getEngine().getQueryPool(),
		11, 1, sizeof(uint64_t),
		&mainSSAOEnd, sizeof(uint64_t),
		VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));

	timeSpentInMainSSAOPass += (mainSSAOEnd - mainSSAOStart) / 1000.0;
#endif

	submitInfo.pWaitSemaphores = &mainSSAOPassCompleteSemaphore;
	submitInfo.pCommandBuffers = &blurSSAOPassCmdBuffer;
	submitInfo.pSignalSemaphores = &blurSSAOPassCompleteSemaphore;

	VK_CHECK(vkQueueSubmit(VkEngine::getEngine().getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
#ifdef PERF_GPU_TIME
	uint64_t blurSSAOStart = 0;
	VK_CHECK(vkGetQueryPoolResults(VkEngine::getEngine().getDevice(), VkEngine::getEngine().getQueryPool(),
		12, 1, sizeof(uint64_t),
		&blurSSAOStart, sizeof(uint64_t),
		VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));

	uint64_t blurSSAOEnd = 0;
	VK_CHECK(vkGetQueryPoolResults(VkEngine::getEngine().getDevice(), VkEngine::getEngine().getQueryPool(),
		13, 1, sizeof(uint64_t),
		&blurSSAOEnd, sizeof(uint64_t),
		VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));

	timeSpentInBlurSSAOPass += (blurSSAOEnd - blurSSAOStart) / 1000.0;
#endif

	submitInfo.pWaitSemaphores = &blurSSAOPassCompleteSemaphore;
	submitInfo.pSignalSemaphores = &lightingPassCompleteSemaphore;
	submitInfo.pCommandBuffers = &lightingPassCmdBuffer;

	VK_CHECK(vkQueueSubmit(VkEngine::getEngine().getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
#ifdef PERF_GPU_TIME
	uint64_t lightingStart = 0;
	VK_CHECK(vkGetQueryPoolResults(VkEngine::getEngine().getDevice(), VkEngine::getEngine().getQueryPool(),
		14, 1, sizeof(uint64_t),
		&lightingStart, sizeof(uint64_t),
		VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));

	uint64_t lightingEnd = 0;
	VK_CHECK(vkGetQueryPoolResults(VkEngine::getEngine().getDevice(), VkEngine::getEngine().getQueryPool(),
		15, 1, sizeof(uint64_t),
		&lightingEnd, sizeof(uint64_t),
		VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));

	timeSpentInLightingPass += (lightingEnd - lightingStart) / 1000.0;
#endif

	submitInfo.pWaitSemaphores = &lightingPassCompleteSemaphore;
	submitInfo.pSignalSemaphores = &sssBlurPassOneCompleteSemaphore;
	submitInfo.pCommandBuffers = &sssBlurPassOneCmdBuffer;

	VK_CHECK(vkQueueSubmit(VkEngine::getEngine().getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
#ifdef PERF_GPU_TIME
	uint64_t sssBlurOneStart = 0;
	VK_CHECK(vkGetQueryPoolResults(VkEngine::getEngine().getDevice(), VkEngine::getEngine().getQueryPool(),
		16, 1, sizeof(uint64_t),
		&sssBlurOneStart, sizeof(uint64_t),
		VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));

	uint64_t sssBlurOneEnd = 0;
	VK_CHECK(vkGetQueryPoolResults(VkEngine::getEngine().getDevice(), VkEngine::getEngine().getQueryPool(),
		17, 1, sizeof(uint64_t),
		&sssBlurOneEnd, sizeof(uint64_t),
		VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));

	timeSpentInSSSPass += (sssBlurOneEnd - sssBlurOneStart) / 1000.0;
#endif

	submitInfo.pWaitSemaphores = &sssBlurPassOneCompleteSemaphore;
	submitInfo.pSignalSemaphores = &sssBlurPassTwoCompleteSemaphore;
	submitInfo.pCommandBuffers = &sssBlurPassTwoCmdBuffer;

	VK_CHECK(vkQueueSubmit(VkEngine::getEngine().getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
#ifdef PERF_GPU_TIME
	uint64_t sssBlurTwoStart = 0;
	VK_CHECK(vkGetQueryPoolResults(VkEngine::getEngine().getDevice(), VkEngine::getEngine().getQueryPool(),
		18, 1, sizeof(uint64_t),
		&sssBlurTwoStart, sizeof(uint64_t),
		VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));

	uint64_t sssBlurTwoEnd = 0;
	VK_CHECK(vkGetQueryPoolResults(VkEngine::getEngine().getDevice(), VkEngine::getEngine().getQueryPool(),
		19, 1, sizeof(uint64_t),
		&sssBlurTwoEnd, sizeof(uint64_t),
		VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));

	timeSpentInSSSPass += (sssBlurTwoEnd - sssBlurTwoStart) / 1000.0;
#endif

	submitInfo.pWaitSemaphores = &sssBlurPassTwoCompleteSemaphore;
	submitInfo.pSignalSemaphores = &renderingCompleteSemaphore;
	submitInfo.pCommandBuffers = &mergePassCmdBuffer;

	VK_CHECK(vkQueueSubmit(VkEngine::getEngine().getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
#ifdef PERF_GPU_TIME
	uint64_t mergeStart = 0;
	VK_CHECK(vkGetQueryPoolResults(VkEngine::getEngine().getDevice(), VkEngine::getEngine().getQueryPool(),
		20, 1, sizeof(uint64_t),
		&mergeStart, sizeof(uint64_t),
		VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));

	uint64_t mergeEnd = 0;
	VK_CHECK(vkGetQueryPoolResults(VkEngine::getEngine().getDevice(), VkEngine::getEngine().getQueryPool(),
		21, 1, sizeof(uint64_t),
		&mergeEnd, sizeof(uint64_t),
		VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));

	timeSpentInMergePass += (mergeEnd - mergeStart) / 1000.0;
#endif
}

VkCommandBuffer GfxPipeline::getPresentationCmdBuffer() const
{ 
	return mergePass->getCurrentCmdBuffer(); 
}

VkRenderPass GfxPipeline::getPresentationRenderPass() const
{
	return mergePass->getRenderPass();
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