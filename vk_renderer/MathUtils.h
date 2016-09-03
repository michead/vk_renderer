#pragma once

#include <glm\glm.hpp>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(a, min_, max_) MIN((max_),MAX((min_),(a)))

struct Frame {
	glm::vec3 origin;
	glm::vec3 xAxis;
	glm::vec3 yAxis;
	glm::vec3 zAxis;

	static Frame lookAtFrame(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up)
	{ 
		Frame frame = {};
		frame.origin = eye; 
		frame.zAxis = normalize(center - eye); 
		
		frame.yAxis = glm::normalize(glm::cross(up, frame.zAxis)); 
		frame.xAxis = glm::cross(frame.yAxis, frame.zAxis); 
		
		return frame; 
	}
};