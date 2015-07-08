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

#include "VolumetricLightRenderer.h"
#include <vxGL/Buffer.h>
#include "../gl/ObjectManager.h"
#include <UniformVolumetricFogBuffer.h>
#include <vxGL/ShaderManager.h>
#include <vxLib/File/FileHandle.h>

namespace Graphics
{
	VolumetricLightRenderer::VolumetricLightRenderer()
	{

	}

	VolumetricLightRenderer::~VolumetricLightRenderer()
	{

	}

	void VolumetricLightRenderer::initialize(vx::StackAllocator* scratchAllocator, const void* p)
	{
		s_shaderManager->loadPipeline(vx::FileHandle("volume.pipe"), "volume.pipe", scratchAllocator);

		UniformVolumetricFogBufferBlock data;
		data.position = vx::float4a(0, 1.5f, 1.5f, 0);
		data.boundsMin = vx::float4a(-1, 0, 0.5f, 0);
		data.boundsMax = vx::float4a(1, 3, 2.5f, 0);

		vx::gl::BufferDescription desc{};
		desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		desc.flags = 0;
		desc.immutable = 1;
		desc.pData = &data;
		desc.size = sizeof(UniformVolumetricFogBufferBlock);
		s_objectManager->createBuffer("volumetricFogBuffer", desc);
	}

	void VolumetricLightRenderer::shutdown()
	{

	}

	void VolumetricLightRenderer::getCommandList(CommandList* cmdList)
	{

	}

	void VolumetricLightRenderer::clearData()
	{

	}

	void VolumetricLightRenderer::bindBuffers()
	{
		auto volumetricFogBuffer = s_objectManager->getBuffer("volumetricFogBuffer");
	}
}