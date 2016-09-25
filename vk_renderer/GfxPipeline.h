#pragma once

#include "vulkan\vulkan.h"


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

	uint16_t getNumPasses() const { return 2; }

private:
	GeometryPass* geometryPass;
	LightingPass* lightingPass;

	VkSemaphore geomPassCompleteSemaphore;
	VkSemaphore finalPassCompleteSemaphore;

	void cleanup();
};