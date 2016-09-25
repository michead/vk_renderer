#include "Quad.h"

#include "VkPool.h"


const std::vector<glm::vec3> Quad::positions = {
	glm::vec3(-1, -1, 0),
	glm::vec3(-1,  1, 0),
	glm::vec3( 1,  1, 0),
	glm::vec3( 1, -1, 0)
};

const std::vector<uint32_t> Quad::indices = { 
	0, 1, 2, 
	2, 3, 0 
};

const std::vector<glm::vec2> Quad::uvs = {
	{ 0, 0 },
	{ 0, 1 },
	{ 1, 1 },
	{ 1, 0 }
};

const std::vector<Vertex> Quad::vertices = {
	Vertex(positions[0], uvs[0]),
	Vertex(positions[1], uvs[1]),
	Vertex(positions[2], uvs[2]),
	Vertex(positions[3], uvs[3])
};


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