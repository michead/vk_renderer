#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 1) uniform sampler2D samplerAlbedo;
layout (binding = 2) uniform sampler2D samplerNormal;
layout (binding = 3) uniform sampler2D samplerSpecular;

layout (binding = 4) uniform Material {
	vec3		kd;
	vec3		ks;
	bool		kdMap;
	bool		ksMap;
	bool		normalMap;
	float		ns;
	float		opacity;
	float		translucency;
	float		subsurfWidth;
} material;

layout (location = 0) in vec4 inColor;
layout (location = 1) in vec3 inPosition;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec3 inNormal;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outPosition;
layout (location = 2) out vec4 outNormal;

mat3 TBN(vec3 position, vec2 texCoord, vec3 normal) {
	vec3 Q1 = dFdx(position);
	vec3 Q2 = dFdy(position);

	vec2 st1 = dFdx(texCoord);
	vec2 st2 = dFdy(texCoord);
 
	vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
	vec3 B = normalize(-Q1 * st2.s + Q2 * st1.s);
 
	return mat3(T, B, normal);
}

void main() 
{
	vec4 albedo = texture(samplerAlbedo, inTexCoord);
	vec3 normal = material.normalMap ? texture(samplerNormal, inTexCoord).xyz : inNormal;
 
	// transform the normal to eye space 
	normal = normal * TBN(inPosition, inTexCoord, normal);


	outPosition = vec4(inPosition, 1);
	outNormal = vec4(2 * normal - vec3(1), 1);
	outColor = vec4(normal.rgb, 1);
}
