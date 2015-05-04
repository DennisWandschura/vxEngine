/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "LightBufferManager.h"
#include "GpuStructs.h"
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
	VX_ASSERT(count <= 5);

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