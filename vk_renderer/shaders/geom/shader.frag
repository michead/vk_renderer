#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 1) uniform sampler2D samplerAlbedo;
layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outFragcolor;


void main() 
{
	vec4 albedo = texture(samplerAlbedo, inUV);

	 outFragcolor = vec4(albedo.rgb, 1.0);	
}
