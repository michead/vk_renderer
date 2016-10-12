#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define RADIUS			0.5
#define KERNEL_SIZE		32

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform ViewUniformBufferObject {
	vec4 noiseScale;
	mat4 view;
	mat4 proj;
} unif;

layout(binding = 1) uniform KernelUniformBufferObject {
	vec4 sampleKernel[KERNEL_SIZE];
} kernel;

layout(binding = 2) uniform sampler2D samplerNoise;
layout(binding = 3) uniform sampler2D samplerNormal;
layout(binding = 4) uniform sampler2D samplerDepth;

mat4 invProj = inverse(unif.proj);

bool isSampleOccluded(vec3 fragVSPos, float fragSSDepth, mat3 tbn, int index) {
	vec3 smpl = tbn * kernel.sampleKernel[index].xyz;
	vec3 vsSmplPos = smpl * RADIUS + fragVSPos;
	vec4 ssSmplPos = unif.proj * vec4(vsSmplPos, 1);
	ssSmplPos.z /= ssSmplPos.w;

	vec4 offset = vec4(vsSmplPos, 1);
	offset = unif.proj * offset;
	offset.xy /= offset.w;
	offset.xy = offset.xy * 0.5 + 0.5;

	float sampleDepth = texture(samplerDepth, offset.xy).r;
	return sampleDepth < ssSmplPos.z && abs(fragSSDepth - sampleDepth) < RADIUS;
}

mat3 tbnMat(vec3 normal) {
	vec3 randVec = texture(samplerNoise, inTexCoord * unif.noiseScale.xy).rgb;
	vec3 tangent = normalize(randVec - normal * dot(randVec, normal));
	vec3 bitangent = normalize(cross(normal, tangent));
	
	return mat3(tangent, bitangent, normal);
}

void main() {
	if (unif.noiseScale.xy == vec2(0)) { 
		outColor = vec4(1, 0, 0, 0);
		return; 
	}

	vec3 normal = (unif.view * texture(samplerNormal, inTexCoord)).rgb;
	mat3 tbn = tbnMat(normal);
	
	float fragSSDepth = texture(samplerDepth, inTexCoord).r;
    vec2 scaledTexCoord = inTexCoord * 2 - 1;
	vec3 pos = vec3(scaledTexCoord.x, scaledTexCoord.y, fragSSDepth);
	vec4 unprojPos = invProj * vec4(pos, 1);
	vec3 fragVSPos = unprojPos.xyz / unprojPos.w;

	float occlusion = 0;

	for (int i = 0; i < KERNEL_SIZE; i++) {
		if (isSampleOccluded(fragVSPos, fragSSDepth, tbn, i)) {
			occlusion++;
		}
	}

	float visibility = 1 - occlusion / KERNEL_SIZE;
   
	outColor.r = visibility;
}