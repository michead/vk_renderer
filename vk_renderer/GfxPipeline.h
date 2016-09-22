#pragma once

#include "vulkan\vulkan.h"


class GeomPass;
class FinalPass;


class GfxPipeline {
public:
	GfxPipeline() { init(); }
	~GfxPipeline() { cleanup(); }

	void init();
	bool run();
	void updateData();

	uint16_t getNumPasses() const { return 2; }

private:
	GeomPass* geomPass;
	FinalPass* finalPass;

	VkSemaphore geomPassCompleteSemaphore;
	VkSemaphore finalPassCompleteSemaphore;

	void cleanup();
};