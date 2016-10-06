#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define BLUR_SIZE 4

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2DArray samplerColor;
layout(binding = 1) uniform sampler2DArray samplerAO;

void main() {
	vec3 color = texture(samplerColor, vec3(inTexCoord, 0)).rgb;
	vec2 texelSize = 1 / vec2(textureSize(samplerAO, 0));
	vec2 offsetOrigin = vec2(-BLUR_SIZE * 0.5 + 0.5);
	
	float avgVisibility = 0;

	for (int i = 0; i < BLUR_SIZE; i++) {
		for (int j = 0; j < BLUR_SIZE; j++) {
			vec2 offset = (offsetOrigin + vec2(i, j)) * texelSize;
			avgVisibility += texture(samplerAO, vec3(inTexCoord + offset, 0)).r;
		}
	}

	avgVisibility /= float(BLUR_SIZE * BLUR_SIZE);
	
	outColor = vec4(avgVisibility * color, 1);
}