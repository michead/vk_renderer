#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"
#include <glm\gtx\vector_angle.hpp>

#include "Frame.h"
#include "MathUtils.h"

#define CAMERA_POSITION		{ 0, 0, 1 }
#define CAMERA_TARGET		{ 0, 0, 0 }
#define CAMERA_UP			{ 0, 1, 0 }
#define CAMERA_FOVY			0.5f
#define CAMERA_NEAR			1.f
#define CAMERA_FAR			10.f
#define CAMERA_ROT_SCALE	.001f
#define CAMERA_DOLLY_SCALE	.001f
#define CAMERA_PAN_SCALE	.001f
#define MIN_THETA			.001f
#define MIN_FOCUS			.00001f


struct CameraUniformBufferObject {
	glm::mat4 view;
	glm::mat4 proj;
};

enum CameraMovement {
	STILL,
	ROTATE,
	ROTATE_AROUND_TARGET,
	PAN,
	ZOOM
};


struct Camera {
public:
	Camera(glm::vec3 position, glm::vec3 up, float fovy, glm::vec3 target) : up(up), fovy(fovy), target(target) { frame.origin = position; }
	~Camera() { }

	Frame frame;
	glm::vec3 up;
	glm::vec3 target;
	float aspectRatio;
	float fovy;
	float focus;
	CameraMovement movement;

	glm::mat4& getViewMatrix() { return viewMatrix; }
	glm::mat4& getProjMatrix() { return projMatrix; }

	void rotate(glm::vec2 rot);
	void rotateAroundTarget(glm::vec2 rot);
	void pan(glm::vec2 pan);
	void zoom(float zoom);

	void initMatrices();
	void updateViewMatrix();

	static void lookAtCamera(
		Camera* camera, 
		const glm::vec3& eye, 
		const glm::vec3& center, 
		const glm::vec3& up)
	{
		camera->frame = Frame::lookAtFrame(eye, center, up);
		camera->focus = length(eye - center);
	}

private:
	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;
};