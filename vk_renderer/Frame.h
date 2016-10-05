#pragma once

#include "MathUtils.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm\glm.hpp>


#define IDENTITY_FRAME {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}


struct Frame {
	glm::vec3 origin;
	glm::vec3 xAxis;
	glm::vec3 yAxis;
	glm::vec3 zAxis;

	static Frame lookAtFrame(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up)
	{
		Frame frame = {};
		frame.origin = eye;
		frame.zAxis = -normalize(center - eye);

		frame.yAxis = glm::normalize(glm::cross(up, frame.zAxis));
		frame.xAxis = glm::cross(frame.yAxis, frame.zAxis);

		return frame;
	}

	static Frame orthonormalizeF(const Frame& f)
	{
		Frame ret = f;
		ret.zAxis = normalize(f.zAxis);
		ret.xAxis = orthonormalize(f.xAxis, ret.zAxis);
		ret.yAxis = normalize(cross(ret.zAxis, ret.xAxis)); 
		return ret;
	}

	glm::mat4 toMatrix() const
	{
		return glm::mat4(
			xAxis.x, yAxis.x, zAxis.x, origin.x,
			xAxis.y, yAxis.y, zAxis.y, origin.y,
			xAxis.z, yAxis.z, zAxis.z, origin.z,
			0, 0, 0, 1);
	}
};
