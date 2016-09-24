#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"
#include <glm\gtx\vector_angle.hpp>

#include "MathUtils.h"

#define CAMERA_POSITION		{ 0, 0, 1 }
#define CAMERA_TARGET		{ 0, 0, 0 }
#define CAMERA_UP			{ 0, 1, 0 }
#define CAMERA_FOVY			45.f
#define CAMERA_NEAR			0.1f
#define CAMERA_FAR			10.f
#define CAMERA_ROT_SCALE	0.001f
#define CAMERA_DOLLY_SCALE	0.001f
#define CAMERA_PAN_SCALE	0.001f
#define MIN_THETA			0.01f
#define MIN_FOCUS			0.00001f

#define CAMERA_FILENAME		"camera.json"

enum CameraMovement {
	STILL,
	ROTATE,
	ROTATE_AROUND_TARGET,
	PAN,
	ZOOM
};

struct Camera {
public:
	Frame frame;
	glm::vec3 target;
	float aspectRatio;
	float fovy;
	CameraMovement movement;

	glm::mat4& getViewMatrix() { return viewMatrix; }
	glm::mat4& getProjMatrix() { return projMatrix; }

	float getFocus() { return MAX(MIN_FOCUS, glm::distance(frame.origin, target)); }

	void rotate(glm::vec2 rot);
	void rotateAroundTarget(glm::vec2 rot);
	void pan(glm::vec2 pan);
	void zoom(float zoom);

	void updateMatrices();

private:
	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;

	bool isDirty = true;
};