#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define PI 3.14159265359

#define MAX_NUM_LIGHTS		4

layout(binding = 0) uniform sampler2D samplerColor;
layout(binding = 1) uniform sampler2D samplerPosition;
layout(binding = 2) uniform sampler2D samplerNormal;
layout(binding = 3) uniform sampler2D samplerTangent;
layout(binding = 4) uniform sampler2D samplerSpecular;
layout(binding = 5) uniform sampler2D samplerMaterial;
layout(binding = 6) uniform sampler2D samplerDepth;
layout(binding = 7) uniform Camera {
	vec4 position;
} camera;
layout(binding = 8) uniform Lights {
	vec4 positions[MAX_NUM_LIGHTS];
	vec4 intensities[MAX_NUM_LIGHTS];
	int count;
} lights;
layout(binding = 9) uniform Scene {
	vec4 ambient;
} scene;
layout(binding = 10) uniform sampler2D shadowMaps[MAX_NUM_LIGHTS];

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outColor;

float beckmanToPhong(float alpha) { return 2 / (alpha * alpha) - 2; }

void main() {
	vec3 kd = texture(samplerColor, inTexCoord).rgb;
    vec3 position = texture(samplerPosition, inTexCoord).xyz;
    vec3 normal = texture(samplerNormal, inTexCoord).rgb;
	vec3 tangent = texture(samplerTangent, inTexCoord).rgb;
	vec3 ks = texture(samplerSpecular, inTexCoord).rgb;
	float rs = texture(samplerSpecular, inTexCoord).a;
	float translucency = texture(samplerMaterial, inTexCoord).r;
	float subsurfWidth = texture(samplerMaterial, inTexCoord).b;

	// Hack, review this
	vec3 color = kd * scene.ambient.xyz;

    for(int i = 0; i < lights.count; i++) {
		vec3 lightPosition = lights.positions[i].xyz;
		vec3 lightIntensity = lights.intensities[i].xyz;
        vec3 cl = lightIntensity / pow(length(lightPosition - position), 2);
        vec3 l = normalize(lightPosition - position);
        vec3 v = normalize(camera.position.xyz - position);
        vec3 h = normalize(v + l);
		
		// Add support for speculars
		float n = beckmanToPhong(rs);
        color += cl * max(0.0, dot(l, normal)) * (kd / PI + ks * (n + 8) / (8 * PI) * pow(max(0.0, dot(h, normal)), n));
    }

	float depth = texture(shadowMaps[0], inTexCoord).r;
    outColor = vec4(vec3(depth), 1);
}