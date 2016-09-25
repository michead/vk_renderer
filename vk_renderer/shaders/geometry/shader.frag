#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 1) uniform sampler2D samplerAlbedo;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec3 inNormal;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec3 outPosition;
layout (location = 2) out vec3 outNormal;


void main() 
{
	vec4 albedo = texture(samplerAlbedo, inTexCoord);

	outPosition = inPosition;
	outNormal = inNormal;
	outColor = vec4(albedo.rgb, 1);
}
