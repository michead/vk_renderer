#pragma once

#include "Pass.h"


class ShadowPass : Pass {
	using Pass::Pass;

private:
private:
	virtual void initAttachments() override;
	virtual void initCommandBuffers() override;
	virtual void initFramebuffers() override;
};