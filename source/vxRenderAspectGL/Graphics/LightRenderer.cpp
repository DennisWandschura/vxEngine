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

#include "LightRenderer.h"
#include <vxEngineLib/Light.h>
#include <vxEngineLib/memcpy.h>
#include <vxEngineLib/EngineConfig.h>
#include <vxLib/Graphics/Camera.h>
#include "../gl/ObjectManager.h"
#include "../gl/BufferBindingManager.h"
#include <vxGL/Buffer.h>
#include "../GpuStructs.h"

namespace Graphics
{
	LightRenderer::LightRenderer()
		:m_lights(),
		m_activeLights(),
		m_lightCount(0),
		m_maxActiveLights(0),
		m_activeLightCount(0)
	{

	}

	LightRenderer::~LightRenderer()
	{

	}

	bool LightRenderer::initialize(vx::StackAllocator* scratchAllocator, Logfile* errorlog, const void* p)
	{
		m_maxActiveLights = s_settings->m_rendererSettings.m_maxActiveLights;
		m_activeLights = vx::make_unique<Gpu::LightData[]>(m_maxActiveLights);

		auto gpuBufferSizeInBytes = sizeof(Gpu::LightData) * m_maxActiveLights + sizeof(vx::uint4);
		auto gpuData = vx::make_unique<u8[]>(gpuBufferSizeInBytes);
		memset(gpuData.get(), 0, gpuBufferSizeInBytes);

		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		desc.size = gpuBufferSizeInBytes;
		desc.immutable = 1;
		desc.flags = vx::gl::BufferStorageFlags::Write | vx::gl::BufferStorageFlags::Dynamic_Storage;
		desc.pData = gpuData.get();

		s_objectManager->createBuffer("lightDataBuffer", desc);

		vx::gl::DrawArraysIndirectCommand cmd = {};
		cmd.instanceCount = 1;

		desc.bufferType = vx::gl::BufferType::Draw_Indirect_Buffer;
		desc.pData = &cmd;
		desc.flags = vx::gl::BufferStorageFlags::Write | vx::gl::BufferStorageFlags::Dynamic_Storage| vx::gl::BufferStorageFlags::Read;

		s_objectManager->createBuffer("cmdLight", desc);

		return true;
	}

	void LightRenderer::shutdown()
	{

	}

	void LightRenderer::getCommandList(CommandList* cmdList)
	{

	}

	void LightRenderer::clearData()
	{

	}

	void LightRenderer::bindBuffers()
	{
		auto lightDataBuffer = s_objectManager->getBuffer("lightDataBuffer");

		gl::BufferBindingManager::bindBaseUniform(1, lightDataBuffer->getId());
	}

	void LightRenderer::setLights(const Light* lights, u32 count)
	{
		m_lights = vx::make_unique<Gpu::LightData[]>(count);
		for (u32 i = 0; i < count; ++i)
		{
			auto &light = lights[i];

			Gpu::LightData data;
			data.position = vx::loadFloat3(light.m_position);
			data.falloff = light.m_falloff;
			data.lumen = light.m_lumen;

			m_lights[i] = data;
		}
		m_lightCount = count;

		m_lightDistances = vx::make_unique<std::pair<f32, u32>[]>(count);
	}

	void LightRenderer::cullLights(const vx::Camera &camera)
	{
		auto totalLights = m_lightCount;
		if (totalLights == 0)
			return;

		auto lights = m_lights.get();
		auto lightDistances = m_lightDistances.get();

		auto cameraPosition = _mm256_cvtpd_ps(camera.getPosition());

		for (u32 i = 0;i < totalLights; ++i)
		{
			auto lightPosition = lights[i].position;

			auto distance = _mm_sub_ps(cameraPosition, lightPosition);
			distance = vx::dot3(distance, distance);
			distance = _mm_sqrt_ps(distance);

			_mm_store_ss(&lightDistances[i].first, distance);
			lightDistances[i].second = i;
		}

		std::sort(lightDistances, lightDistances + totalLights, [](const std::pair<f32, u32> &lhs, const std::pair<f32, u32> &rhs)
		{
			return lhs.first < rhs.first;
		});

		auto activeLights = m_activeLights.get();
		auto maxActiveLights = m_maxActiveLights;

		auto activeLightCount = std::min(totalLights, maxActiveLights);

		for (u32 i = 0; i < activeLightCount; ++i)
		{
			auto lightIndex = lightDistances[i].second;

			activeLights[i] = lights[lightIndex];
		}

		m_activeLightCount = activeLightCount;

		auto lightDataSize = sizeof(Gpu::LightData) * activeLightCount;
		auto lightCountOffset = sizeof(Gpu::LightData) * maxActiveLights;

		auto lightDataBuffer = s_objectManager->getBuffer("lightDataBuffer");
		auto lightCmdBuffer = s_objectManager->getBuffer("cmdLight");

		auto mappedBuffer = lightDataBuffer->mapRange<Gpu::LightData>(0, lightDataSize, vx::gl::MapRange::Write);
		memcpy(mappedBuffer.get(), activeLights, lightDataSize);
		mappedBuffer.unmap();

		auto mappedBufferCount = lightDataBuffer->mapRange<u32>(lightCountOffset, sizeof(u32), vx::gl::MapRange::Write);
		memcpy(mappedBufferCount.get(), &activeLightCount, sizeof(u32));
		mappedBufferCount.unmap();

		auto mappedCmdBuffer = lightCmdBuffer->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
		mappedCmdBuffer->count = activeLightCount;
		//lightCmdBuffer->subData(0, sizeof(u32), &activeLightCount);

		//lightDataBuffer->subData(0, lightDataSize, activeLights);
		//lightDataBuffer->subData(lightCountOffset, sizeof(u32), &activeLightCount);
	}
}