#include "SceneElem.h"

#include "VkUtils.h"
#include "VkPool.h"


void SceneElem::initVertexBuffer()
{
	BufferData bufferData = VkEngine::getEngine().getPool()->createVertexBuffer(mesh.vertices);
	vertexBuffer = bufferData.buffer;
	vertexBufferMemory = bufferData.bufferMemory;
}

void SceneElem::initIndexBuffer()
{
	BufferData bufferData = VkEngine::getEngine().getPool()->createIndexBuffer(mesh.indices);
	indexBuffer = bufferData.buffer;
	indexBufferMemory = bufferData.bufferMemory;
}