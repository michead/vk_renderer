#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 1) uniform sampler2D samplerColor;
layout (binding = 2) uniform sampler2D samplerNormal;

layout (binding = 3) uniform Material {
	vec3		kd;
	vec3		ks;
	float		rs;
	float		opacity;
	float		translucency;
	float		subsurfWidth;
} material;

layout (location = 0) in vec4 inColor;
layout (location = 1) in vec3 inPosition;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec3 inNormal;
layout (location = 4) in vec3 inTangent;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outPosition;
layout (location = 2) out vec4 outNormal;
layout (location = 3) out vec4 outTangent;
layout (location = 4) out vec4 outSpecular;
layout (location = 5) out vec4 outMaterial;

void main() 
{
	vec3 color = texture(samplerColor, inTexCoord).xyz;

	outColor = vec4(color, 1);
	outPosition = vec4(inPosition, 1);
	outNormal = vec4(inNormal, 0);
	outTangent = vec4(inTangent, 0);
	outSpecular = vec4(material.ks, material.rs);
	outMaterial = vec4(material.translucency, material.subsurfWidth, 0, 0);
}
