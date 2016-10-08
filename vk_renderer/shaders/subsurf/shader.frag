#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define NUM_SAMPLES	17

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D samplerColor;
layout(binding = 1) uniform sampler2D samplerDepth;
layout(binding = 2) uniform sampler2D samplerMaterial;
layout(binding = 3) uniform Camera {
	float fovy;
} camera;
layout(binding = 4) uniform Instance {
	vec4 kernel[NUM_SAMPLES];
	vec2 blurDirection;
} instance;

void main() {
	vec4 colorM = texture(samplerColor, inTexCoord);
	
	if (camera.fovy == 0.f) { 
		outColor = vec4(colorM.rgb, 1);
		return; 
	}
	
	float depthM = texture(samplerDepth, inTexCoord).r;
	float subsurfWidth = texture(samplerMaterial, inTexCoord).g;

	float dist = 1.0 / tan(0.5 * camera.fovy);
    float scale = dist / depthM;

    vec2 finalStep = subsurfWidth * scale * instance.blurDirection;
    finalStep *= colorM.a;
    finalStep *= 1.0f / 3.0f;

    vec3 colorBlurred = colorM.xyz;
    colorBlurred *= instance.kernel[0].rgb;

    for (int i = 1; i < NUM_SAMPLES; i++) {
        vec2 offset = inTexCoord + instance.kernel[i].a * finalStep;
        vec3 color = texture(samplerColor, offset).rgb;

        float depth = texture(samplerDepth, offset).r;
        float s = clamp(300.0f * dist * subsurfWidth * abs(depthM - depth), 0.0f, 1.0f);
        color = mix(color, colorM.xyz, s);

        colorBlurred += instance.kernel[i].rgb * color;
    }

	outColor = vec4(colorBlurred, 1);
}
