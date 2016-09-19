#include "Quad.h"

#include "VkPool.h"


std::vector<Vertex> Quad::vertices = {
	glm::vec3(0, 0, 0),
	glm::vec3(0, 1, 0),
	glm::vec3(1, 1, 0),
	glm::vec3(1, 0, 0) };

std::vector<uint32_t> Quad::indices = { 0, 1, 2, 2, 3, 0 };

void Quad::initVertexBuffer()
{
	BufferData bufferData = VkEngine::getEngine().getPool()->createVertexBuffer(vertices);
	vertexBuffer = bufferData.buffer;
	vertexBufferMemory = bufferData.bufferMemory;
}

void Quad::initIndexBuffer()
{
	BufferData bufferData = VkEngine::getEngine().getPool()->createIndexBuffer(indices);
	indexBuffer = bufferData.buffer;
	indexBufferMemory = bufferData.bufferMemory;
}