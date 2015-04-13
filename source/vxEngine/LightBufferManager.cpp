#include "LightBufferManager.h"
#include "BufferBlocks.h"
#include "BufferBindingManager.h"
#include "Light.h"

void LightBufferManager::initialize(U32 maxLightCount)
{
	createLightDataBuffer(maxLightCount);
	m_maxLightCount = maxLightCount;
}

void LightBufferManager::createLightDataBuffer(U32 maxLightCount)
{
	LightDataBlock lightdata;
	lightdata.size = 0;

	vx::gl::BufferDescription lightDataDesc;
	lightDataDesc.bufferType = vx::gl::BufferType::Uniform_Buffer;
	lightDataDesc.size = sizeof(LightDataBlock);
	lightDataDesc.immutable = 1;
	lightDataDesc.flags = vx::gl::BufferStorageFlags::Write;
	lightDataDesc.pData = &lightdata;
	m_lightDataBuffer.create(lightDataDesc);
}

void LightBufferManager::bindBuffer()
{
	BufferBindingManager::bindBaseUniform(1, m_lightDataBuffer.getId());
}

void LightBufferManager::updateLightDataBuffer(const Light* lights, U32 count)
{
	VX_ASSERT(count <= 10);

	LightDataBlock data;
	for (auto i = 0u; i < count; ++i)
	{
		data.u_lightData[i].position = lights[i].m_position;
		data.u_lightData[i].falloff = lights[i].m_falloff;
		data.u_lightData[i].direction = lights[i].m_direction;
		data.u_lightData[i].lumen = lights[i].m_lumen;
	}

	data.size = count;

	m_lightCount = count;

	auto lightDataMappedBuffer = m_lightDataBuffer.map<LightDataBlock>(vx::gl::Map::Write_Only);
	*lightDataMappedBuffer = data;
	lightDataMappedBuffer.unmap();
}