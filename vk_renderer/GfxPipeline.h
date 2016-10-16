#pragma once

#include "vulkan\vulkan.h"


#define SHADOW_PASS_VS		"shaders/shadow/vert.spv"
#define SHADOW_PASS_FS		"shaders/shadow/frag.spv"
#define GEOMETRY_PASS_VS	"shaders/geometry/vert.spv"
#define GEOMETRY_PASS_FS	"shaders/geometry/frag.spv"
#define SSAO_MAIN_PASS_VS	"shaders/ssao-main/vert.spv"
#define SSAO_MAIN_PASS_FS	"shaders/ssao-main/frag.spv"
#define SSAO_BLUR_PASS_VS	"shaders/ssao-blur/vert.spv"
#define SSAO_BLUR_PASS_FS	"shaders/ssao-blur/frag.spv"
#define LIGHTING_PASS_VS	"shaders/lighting/vert.spv"
#define LIGHTING_PASS_FS	"shaders/lighting/frag.spv"
#define SUBSURF_PASS_VS		"shaders/subsurf/vert.spv"
#define SUBSURF_PASS_FS		"shaders/subsurf/frag.spv"
#define MERGE_PASS_VS		"shaders/merge/vert.spv"
#define MERGE_PASS_FS		"shaders/merge/frag.spv"

#define FENCE_TIMEOUT		100000


class ShadowPass;
class GeometryPass;
class LightingPass;
class SSAOPass;
class SubsurfPass;
class MergePass;


class GfxPipeline {
public:
	GfxPipeline() { init(); }
	~GfxPipeline() { cleanup(); }

	void init();
	void run();
	void initBufferData();
	void updateBufferData();

	VkRenderPass getPresentationRenderPass() const;
	VkCommandBuffer getPresentationCmdBuffer() const;

	long long getTimeSpentInShadowPass() { return timeSpentInShadowPass; }
	long long getTimeSpentInGeomPass() { return timeSpentInGeomPass; }
	long long getTimeSpentInMainSSAOPass() { return timeSpentInMainSSAOPass; }
	long long getTimeSpentInBlurSSAOPass() { return timeSpentInBlurSSAOPass; }
	long long getTimeSpentInLightingPass() { return timeSpentInLightingPass; }
	long long getTimeSpentInSSSPass() { return timeSpentInSSSPass; }
	long long getTimeSpentInMergePass() { return timeSpentInMergePass; }

private:
	ShadowPass* shadowPass;
	GeometryPass* geometryPass;
	SSAOPass* ssaoPass;
	LightingPass* lightingPass;
	SubsurfPass* sssBlurPassOne;
	SubsurfPass* sssBlurPassTwo;
	MergePass* mergePass;

	VkSemaphore shadowPassCompleteSemaphore;
	VkSemaphore geomPassCompleteSemaphore;
	VkSemaphore mainSSAOPassCompleteSemaphore;
	VkSemaphore blurSSAOPassCompleteSemaphore;
	VkSemaphore lightingPassCompleteSemaphore;
	VkSemaphore sssBlurPassOneCompleteSemaphore;
	VkSemaphore sssBlurPassTwoCompleteSemaphore;
	VkSemaphore mergePassCompleteSemaphore;

	VkQueryPool queryPool;

	double timeSpentInShadowPass = 0;
	double timeSpentInGeomPass = 0;
	double timeSpentInLightingPass = 0;
	double timeSpentInMainSSAOPass = 0;
	double timeSpentInBlurSSAOPass = 0;
	double timeSpentInSSSPass = 0;
	double timeSpentInMergePass = 0;

	void cleanup();
};