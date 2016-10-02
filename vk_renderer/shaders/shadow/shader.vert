#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform Camera {
	mat4 view;
	mat4 proj;
} camera;
layout(binding = 1) uniform Mesh {
	mat4 model;
} mesh;

layout(location = 1) in vec3 inPosition;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
	gl_Position = camera.proj * camera.view * mesh.model * vec4(inPosition, 1);
}