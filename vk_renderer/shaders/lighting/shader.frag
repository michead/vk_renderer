#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define PI 3.14159265359

#define MAX_NUM_LIGHTS	4

#define TRANSMIT_INV_SCALE	180.f
#define SHRINKING_SCALE		.005f

struct Light {
	vec4 pos;
	vec4 ke;
	mat4 mat;
};

layout(binding = 0) uniform sampler2D samplerColor;
layout(binding = 1) uniform sampler2D samplerPosition;
layout(binding = 2) uniform sampler2D samplerNormal;
layout(binding = 3) uniform sampler2D samplerTangent;
layout(binding = 4) uniform sampler2D samplerSpecular;
layout(binding = 5) uniform sampler2D samplerMaterial;
layout(binding = 6) uniform sampler2D samplerDepth;
layout(binding = 7) uniform Camera {
	vec4 pos;
} camera;
layout(binding = 8) uniform Scene {
	vec4 ka;
	Light lights[MAX_NUM_LIGHTS];
	int numLights;
	bool addSpeculars;
} scene;
layout(binding = 9) uniform sampler2D samplerShadows[MAX_NUM_LIGHTS];
layout(binding = 10) uniform sampler2D samplerVisibility;

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outSpeculars;

vec3 convolute(float scaledDist) {
	float dd = -scaledDist * scaledDist;

	return	vec3(0.233f, 0.455f, 0.649f) * exp(dd / 0.0064f) +
			vec3(0.1f,   0.336f, 0.344f) * exp(dd / 0.0484f) +
			vec3(0.118f, 0.198f, 0.0f)   * exp(dd / 0.187f)  +
			vec3(0.113f, 0.007f, 0.007f) * exp(dd / 0.567f)  +
			vec3(0.358f, 0.004f, 0.0f)   * exp(dd / 1.99f)   +
			vec3(0.078f, 0.0f,   0.0f)   * exp(dd / 7.41f);
}

vec3 transmittance(vec3 pos, vec3 norm, vec3 l, mat4 lightMat, sampler2D shadowMap, float translucency, float subsurfWidth) {
	float scale = TRANSMIT_INV_SCALE * (1.f - translucency) / subsurfWidth;
	vec4 shadowPos = lightMat * vec4(pos - norm * SHRINKING_SCALE, 1);
	vec3 shadowCoords = shadowPos.xyz / shadowPos.w;

	float d1 = texture(shadowMap, shadowCoords.xy * 0.5 + 0.5).r;
	float d2 = shadowCoords.z;
	float scaledDist = scale * abs(d1 - d2);

	vec3 profile = convolute(scaledDist);

	return profile * clamp(0.3f + dot(l, -norm), 0.f, 1.f);
}

void main() {
	vec3 kd = texture(samplerColor, inTexCoord).rgb;
    vec3 position = texture(samplerPosition, inTexCoord).xyz;
    vec3 normal = texture(samplerNormal, inTexCoord).rgb;
	vec3 tangent = texture(samplerTangent, inTexCoord).rgb;
	vec3 ks = texture(samplerSpecular, inTexCoord).rgb;
	float ns = texture(samplerSpecular, inTexCoord).a;
	float translucency = texture(samplerMaterial, inTexCoord).r;
	float subsurfWidth = texture(samplerMaterial, inTexCoord).g;
	float visibility = texture(samplerVisibility, inTexCoord).r;
	vec3 color = kd * scene.ka.rgb;
	vec3 speculars = vec3(0);

    for(int i = 0; i < scene.numLights; i++) {
		vec3 lightPos = scene.lights[i].pos.xyz;
		vec3 lightKe = scene.lights[i].ke.rgb / pow(length(lightPos - position), 2);
		mat4 lightMat = scene.lights[i].mat;
        vec3 lightVec = normalize(lightPos - position);
        vec3 viewVec = normalize(camera.pos.xyz - position);
        vec3 h = normalize(viewVec + lightVec);
		vec3 lightScale = lightKe * max(0.0, dot(lightVec, normal));
		vec3 kt = subsurfWidth > 0 ? 
			transmittance(position, normal, lightVec, lightMat, samplerShadows[i], translucency, subsurfWidth) : vec3(0);
		speculars += lightScale * (ks * (ns + 8) / (8 * PI) * pow(max(0.0, dot(h, normal)), ns));
		vec3 lightMult = lightScale * (kd / PI) + lightKe * kt;
		color += lightMult;
    }

	if (scene.addSpeculars) {
		color + speculars;
	}

	outColor = vec4(color * visibility, 1);
	outSpeculars = vec4(speculars * visibility, 1);
}