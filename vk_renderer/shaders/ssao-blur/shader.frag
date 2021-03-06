#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define BLUR_SIZE 4

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D samplerAO;

void main() {
	vec2 texelSize = 1 / vec2(textureSize(samplerAO, 0));
	vec2 offsetOrigin = vec2(-BLUR_SIZE * 0.5 + 0.5);

	float avgVisibility = 0;

	for (int i = 0; i < BLUR_SIZE; i++) {
		for (int j = 0; j < BLUR_SIZE; j++) {
			vec2 offset = (offsetOrigin + vec2(i, j)) * texelSize;
			avgVisibility += texture(samplerAO, inTexCoord + offset).r;
		}
	}

	avgVisibility /= float(BLUR_SIZE * BLUR_SIZE);
	
	outColor.r = avgVisibility;
}