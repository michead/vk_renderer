#pragma once

#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm\glm.hpp>


#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(a, min_, max_) MIN((max_),MAX((min_),(a)))


inline static glm::mat4 lookAtMatrix(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up)
{
	glm::vec3 w = normalize(eye - center); 
	glm::vec3 u = normalize(cross(up, w)); 
	glm::vec3 v = cross(w, u); 
	
	return glm::mat4(
		u.x, u.y, u.z, -dot(u, eye), 
		v.x, v.y, v.z, -dot(v, eye), 
		w.x, w.y, w.z, -dot(w, eye), 
		0,	   0,	0,			 1); }

inline glm::vec3 orthonormalize(const glm::vec3& a, const glm::vec3& b)
{
	return normalize(a - b*dot(a, b));
}

inline glm::vec3 computeTriangleNormal(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2)
{ 
	return normalize(cross(v1 - v0, v2 - v0));
}

inline float computeTriangleArea(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2)
{
	return length(cross(v2 - v0, v2 - v1)) / 2;
}

inline glm::vec3 computeTangentsFromUVs(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
	const glm::vec2& uv0, const glm::vec2& uv1, const glm::vec2& uv2)
{
	auto p = v1 - v0;
	auto q = v2 - v0;
	auto s = glm::vec2(uv1.x - uv0.x, uv2.x - uv0.x);
	auto t = glm::vec2(uv1.y - uv0.y, uv2.y - uv0.y);
	auto div = s.x*t.y - s.y*t.x;

	if (div > 0) return glm::vec3{ t.y * p.x - t.x * q.x, t.y * p.y - t.x * q.y, t.y * p.z - t.x * q.z } / div;
	else return glm::vec3(1, 0, 0);
}

inline std::vector<float> fComputeVertexNormals(size_t size, size_t fSize, const float* positions, const int* triangles)
{
	std::vector<glm::vec3> normals(size);

	for (size_t f = 0; f < fSize; f += 3)
	{
		int i1 = triangles[f];
		int i2 = triangles[f + 1];
		int i3 = triangles[f + 2];

		auto a = glm::vec3(positions[3 * i1], positions[3 * i1 + 1], positions[3 * i1 + 2]);
		auto b = glm::vec3(positions[3 * i2], positions[3 * i2 + 1], positions[3 * i2 + 2]);
		auto c = glm::vec3(positions[3 * i3], positions[3 * i3 + 1], positions[3 * i3 + 2]);
		auto fn = computeTriangleNormal(a, b, c);
		auto area = computeTriangleArea(a, b, c);

		normals[i1] += area * fn;
		normals[i2] += area * fn;
		normals[i3] += area * fn;
	}

	for (auto& n : normals)
	{
		n = normalize(n);
	}

	std::vector<float> fNormals;

	for (const auto& n : normals)
	{
		fNormals.push_back(n.x);
		fNormals.push_back(n.y);
		fNormals.push_back(n.z);
	}

	return fNormals;
}

inline std::vector<float> fComputeTangents(
	size_t size,
	size_t fSize,
	const float* positions,
	const float* normals,
	const float* texCoords,
	const int* triangles)
{
	std::vector<glm::vec3> tangents(size);

	for (size_t f = 0; f < fSize; f += 3)
	{
		int i1 = triangles[f];
		int i2 = triangles[f + 1];
		int i3 = triangles[f + 2];

		auto a = glm::vec3(positions[3 * i1], positions[3 * i1 + 1], positions[3 * i1 + 2]);
		auto b = glm::vec3(positions[3 * i2], positions[3 * i2 + 1], positions[3 * i2 + 2]);
		auto c = glm::vec3(positions[3 * i3], positions[3 * i3 + 1], positions[3 * i3 + 2]);

		auto ta = glm::vec2(texCoords[2 * i1], texCoords[2 * i1 + 1]);
		auto tb = glm::vec2(texCoords[2 * i1], texCoords[2 * i1 + 1]);
		auto tc = glm::vec2(texCoords[2 * i2], texCoords[2 * i2 + 1]);

		auto ft = computeTangentsFromUVs(a, b, c, ta, tb, tc);
		auto area = computeTriangleArea(a, b, c);
		
		tangents[i1] += area * ft;
		tangents[i2] += area * ft;
		tangents[i3] += area * ft;
	}

	std::vector<glm::vec3> fNormals;
	for (size_t i = 0; i < 3 * size; i += 3)
	{
		fNormals.push_back(glm::vec3(normals[i], normals[i + 1], normals[i + 2]));
	}

	for (size_t i = 0; i < tangents.size(); i++)
	{
		tangents[i] = orthonormalize(tangents[i], fNormals[i]);
	}

	std::vector<float> fTangents;

	for (const auto& t : tangents)
	{
		fTangents.push_back(t.x);
		fTangents.push_back(t.y);
		fTangents.push_back(t.z);
	}

	return fTangents;
}

inline std::vector<glm::vec3> computeVertexNormals(const std::vector<glm::vec3>& positions, const std::vector<glm::ivec3>& triangles)
{
	auto norm = std::vector<glm::vec3>(positions.size());

	for (auto f : triangles)
	{
		auto fn = computeTriangleNormal(positions[f.x], positions[f.y], positions[f.z]);
		auto a = computeTriangleArea(positions[f.x], positions[f.y], positions[f.z]);
		for (int v = 0; v < 3; v++)
		{
			norm[f[v]] += a * fn;
		}
	}

	for (auto& n : norm)
	{
		n = normalize(n);
	}

	return norm;
}

inline std::vector<glm::vec3> computeTangents(
	const std::vector<glm::vec3>& positions,
	const std::vector<glm::vec3>& normals,
	const std::vector<glm::vec2>& texCoords,
	const std::vector<glm::ivec3>& triangles)
{
	auto tangents = std::vector<glm::vec3>(positions.size());
	for (auto f : triangles)
	{
		auto ft = computeTangentsFromUVs(positions[f.x], positions[f.y], positions[f.z], texCoords[f.x], texCoords[f.y], texCoords[f.z]);
		auto area = computeTriangleArea(positions[f.x], positions[f.y], positions[f.z]);
		for (int v = 0; v < 3; v++)
		{
			tangents[f[v]] += area * ft;
		}
	}
	for (size_t i = 0; i < positions.size(); i++)
	{
		tangents[i] = orthonormalize(tangents[i], normals[i]);
	}

	return tangents;
}

template<typename T>
inline static void loadVec(FILE* f, std::vector<T>& v)
{
	auto n = (int) 0;
	fread(&n, sizeof(n), 1, f);
	v.resize(n);
	if (!n) return;
	fread(v.data(), sizeof(T), n, f);
}