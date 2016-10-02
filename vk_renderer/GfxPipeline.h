#pragma once

#include "vulkan\vulkan.h"

class ShadowPass;
class GeometryPass;
class LightingPass;


class GfxPipeline {
public:
	GfxPipeline() { init(); }
	~GfxPipeline() { cleanup(); }

	void init();
	void run();
	void initBufferData();
	void updateBufferData();

	uint16_t getNumPasses() const { return 3; }

private:
	ShadowPass* shadowPass;
	GeometryPass* geometryPass;
	LightingPass* lightingPass;

	VkSemaphore shadowPassCompleteSemaphore;
	VkSemaphore geomPassCompleteSemaphore;
	VkSemaphore finalPassCompleteSemaphore;

	void cleanup();
};