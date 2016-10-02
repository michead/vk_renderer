#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform Camera {
	mat4 view;
	mat4 proj;
} camera;
layout(binding = 1) uniform Mesh {
	mat4 model;
} mesh;

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec3 inPosition;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec3 outPosition;
layout(location = 2) out vec2 outTexCoord;
layout(location = 3) out vec3 outNormal;
layout(location = 4) out vec3 outTangent;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
	outColor = inColor;
	outPosition = inPosition;
	outTexCoord = inTexCoord;
	outNormal = inNormal;
	outTangent = inTangent;

    gl_Position = camera.proj * camera.view * mesh.model * vec4(inPosition, 1);
}