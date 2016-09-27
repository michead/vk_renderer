#include "Mesh.h"

#include "VkUtils.h"
#include "VkPool.h"


void Mesh::initVertexBuffer()
{
	BufferData bufferData = VkEngine::getEngine().getPool()->createVertexBuffer(vertices);
	vertexBuffer = bufferData.buffer;
	vertexBufferMemory = bufferData.bufferMemory;
}

void Mesh::initIndexBuffer()
{
	BufferData bufferData = VkEngine::getEngine().getPool()->createIndexBuffer(indices);
	indexBuffer = bufferData.buffer;
	indexBufferMemory = bufferData.bufferMemory;
}