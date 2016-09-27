#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define PI 3.14159265359

#define POINT_LIGHT_TYPE	0
#define MAX_NUM_LIGHTS		8

layout(binding = 0) uniform sampler2D samplerColor;
layout(binding = 1) uniform sampler2D samplerPosition;
layout(binding = 2) uniform sampler2D samplerNormal;
layout(binding = 3) uniform sampler2D samplerDepth;

layout(binding = 4) uniform Camera {
	vec3 position;
} camera;

layout(binding = 5) uniform Lights {
	int count;
	vec3 positions[MAX_NUM_LIGHTS];
	vec3 intensities[MAX_NUM_LIGHTS];
} lights;

layout(location = 0) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

float beckmanToPhong(float alpha) { return 2 / (alpha * alpha) - 2; }

void main() {
    vec2 texCoord = inTexCoord;
	vec3 color = texture(samplerColor, texCoord).rgb;
    vec3 position = texture(samplerPosition, texCoord).xyz;
    vec3 normal = normalize(texture(samplerNormal, texCoord).rgb);

	// Hack, review this
	vec3 kd = color;

    for(int i = 0; i < lights.count; i++) {
		vec3 lightPosition = lights.positions[i];
		vec3 lightIntensity = lights.intensities[i];
        vec3 cl = lightIntensity / pow(length(lightPosition - position), 2);
        vec3 l = normalize(lightPosition - position);
        vec3 v = normalize(camera.position - position);
        vec3 h = normalize(v + l);
        color += cl * max(0.0, dot(l, normal)) * (kd / PI);

		// Add support for speculars
		// float material_n = beckmanToPhong(rs);
		// if(ks == 0) color += cl * max(0, dot(l, norm)) * (kd / pi);
        // else color += cl * max(0, dot(l, norm)) * (kd / pi + ks * (materialN+8)/(8*pi) * pow(max(0, dot(h, normal)), materialN));
    }

    outColor = vec4(color, 1);
}