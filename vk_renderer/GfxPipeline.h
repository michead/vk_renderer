#pragma once

#include "vulkan\vulkan.h"

class ShadowPass;
class GeometryPass;
class LightingPass;
class SSAOPass;


class GfxPipeline {
public:
	GfxPipeline() { init(); }
	~GfxPipeline() { cleanup(); }

	void init();
	void run();
	void initBufferData();
	void updateBufferData();

	uint16_t getNumPasses() const { return 4; }

private:
	ShadowPass* shadowPass;
	GeometryPass* geometryPass;
	SSAOPass* ssaoPass;
	LightingPass* lightingPass;

	VkSemaphore shadowPassCompleteSemaphore;
	VkSemaphore geomPassCompleteSemaphore;
	VkSemaphore mainSSAOPassCompleteSemaphore;
	VkSemaphore blurSSAOPassCompleteSemaphore;
	VkSemaphore finalPassCompleteSemaphore;

	void cleanup();
};