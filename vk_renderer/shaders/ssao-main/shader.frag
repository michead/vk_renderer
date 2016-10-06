#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define RADIUS			0.1
#define KERNEL_SIZE		32

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform CameraUniformBufferObject {
	vec4 noiseScale;
	mat4 view;
	mat4 proj;
} camera;

layout(binding = 1) uniform KernelUniformBufferObject {
	vec4 sampleKernel[KERNEL_SIZE];
} kernel;

layout(binding = 2) uniform sampler2D samplerNoise;
layout(binding = 3) uniform sampler2D samplerNormal;
layout(binding = 4) uniform sampler2D samplerDepth;


mat4 invProj = inverse(camera.proj);

vec3 vsPos(vec2 texCoord) {
	float z = texture(samplerDepth, texCoord).r;
    vec2 scaledTexCoord = texCoord * 2 - 1;

	vec3 pos = vec3(scaledTexCoord.x, scaledTexCoord.y, z);
	vec4 unprojPos = invProj * vec4(pos, 1);
	
	return unprojPos.xyz / unprojPos.w;
}

bool isSampleOccluded(vec3 origin, mat3 tbn, int index) {
	vec3 smpl = tbn * kernel.sampleKernel[index].xyz;
	smpl = smpl * RADIUS + origin;

	vec4 offset = vec4(smpl, 1);
	offset = camera.proj * offset;
	offset.xy /= offset.w;
	offset.xy = offset.xy * 0.5 + 0.5;
  
	float sampleDepth = vsPos(offset.xy).z;
	return sampleDepth > smpl.z && abs(origin.z - sampleDepth) < RADIUS;
}

mat3 tbnMat(vec3 normal) {
	vec3 randVec = texture(samplerNoise, inTexCoord * camera.noiseScale.xy).rgb;
	vec3 tangent = normalize(randVec - normal * dot(randVec, normal));
	vec3 bitangent = normalize(cross(normal, tangent));
	
	return mat3(tangent, bitangent, normal);
}

void main() {
	vec3 normal = (camera.view * texture(samplerNormal, inTexCoord)).rgb;
	mat3 tbn = tbnMat(normal);

	float occlusion = 0;

	for (int i = 0; i < KERNEL_SIZE; i++) {
		if (isSampleOccluded(vsPos(inTexCoord), tbn, i)) {
			occlusion++;
		}
	}

	float visibility = 1 - occlusion / KERNEL_SIZE;
   
	outColor = vec4(visibility, 0, 0, 0);
}