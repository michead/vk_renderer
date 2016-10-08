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

	VkCommandBuffer getPresentationCmdBuffer() const;

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

	void cleanup();
};