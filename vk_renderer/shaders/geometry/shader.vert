#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform Camera {
	mat4 model;
	mat4 view;
	mat4 proj;
} camera;

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec3 inPosition;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec3 outPosition;
layout(location = 2) out vec2 outTexCoord;
layout(location = 3) out vec3 outNormal;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
	outColor = inColor;
	outPosition = inPosition;
	outTexCoord = inTexCoord;
	outNormal = inNormal;

    gl_Position = camera.proj * camera.view * camera.model * vec4(inPosition, 1);
}