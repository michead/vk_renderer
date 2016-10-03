#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define PI 3.14159265359

#define MAX_NUM_LIGHTS	4

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
} scene;
layout(binding = 9) uniform sampler2D shadowMaps[MAX_NUM_LIGHTS];

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outColor;

void transmittance() {
	
}

void main() {
	vec3 kd = texture(samplerColor, inTexCoord).rgb;
    vec3 position = texture(samplerPosition, inTexCoord).xyz;
    vec3 normal = texture(samplerNormal, inTexCoord).rgb;
	vec3 tangent = texture(samplerTangent, inTexCoord).rgb;
	vec3 ks = texture(samplerSpecular, inTexCoord).rgb;
	float ns = texture(samplerSpecular, inTexCoord).a;
	float translucency = texture(samplerMaterial, inTexCoord).r;
	float subsurfWidth = texture(samplerMaterial, inTexCoord).b;

	// Hack, review this
	vec3 color = kd * scene.ka.rgb;

    for(int i = 0; i < scene.numLights; i++) {
		vec3 lightPos = scene.lights[i].pos.xyz;
		vec3 lightKe = scene.lights[i].ke.rgb / pow(length(lightPos - position), 2);
        vec3 lightVec = normalize(lightPos - position);
        vec3 viewVec = normalize(camera.pos.xyz - position);
        vec3 h = normalize(viewVec + lightVec);
		
        color += lightKe * max(0.0, dot(lightVec, normal)) * (kd / PI + ks * (ns + 8) / (8 * PI) * pow(max(0.0, dot(h, normal)), ns));
    }

    outColor = vec4(color, 1);
}