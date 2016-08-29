#pragma once

#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"
#include <glm\gtx\vector_angle.hpp>

#define CAMERA_POSITION { 0, 1, 3 }
#define CAMERA_TARGET	{ 0, 0, 0 }
#define CAMERA_FORWARD	{ 0, 0, -1 }
#define CAMERA_UP		{ 0, 1, 0 }
#define CAMERA_FOVY		45.f
#define CAMERA_NEAR		0.1f
#define CAMERA_FAR		10.f

class Camera {
public:
	glm::vec3 position;
	glm::quat rotation;
	float aspectRatio;
	float fovy;

	glm::mat4& getViewMatrix() { return viewMatrix; }
	glm::mat4& getProjMatrix() { return projMatrix; }

	static glm::quat getRotationToTarget(glm::vec3 target);

	void rotateCamera(glm::vec3 rot);
	void rotatateCameraAroundTarget(glm::vec3 target, glm::vec3 rot);
	void panCamera(glm::vec2 pan);
	void zoomCamera(float zoom);

	void updateMatrices();

private:
	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;

	bool isDirty = true;
};