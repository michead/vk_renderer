#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D samplerColor;
layout(binding = 1) uniform sampler2D samplerSpeculars;

void main() {
	vec3 diffuse = texture(samplerColor, inTexCoord).rgb;
	vec3 speculars = texture(samplerSpeculars, inTexCoord).rgb;

	outColor = vec4(diffuse + speculars, 1);
}