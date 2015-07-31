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
#include <vxGL/Texture.h>
#include <vxGL/Framebuffer.h>
#include "Commands.h"
#include "Segment.h"
#include "State.h"
#include <vxGL/VertexArray.h>
#include <vxGL/ProgramPipeline.h>
#include <vxgl/gl.h>
#include "CommandList.h"
#include <vxEngineLib/EngineConfig.h>
#include "../GpuStructs.h"

namespace Graphics
{
	VolumetricLightRenderer::VolumetricLightRenderer()
	{

	}

	VolumetricLightRenderer::~VolumetricLightRenderer()
	{

	}

	void VolumetricLightRenderer::createTexture()
	{
		auto halfRes = s_settings->m_resolution / 2u;

		vx::gl::TextureDescription desc;
		desc.format = vx::gl::TextureFormat::RGBA16F;
		desc.miplevels = 1;
		desc.size = vx::ushort3(halfRes.x, halfRes.y, 1);
		desc.sparse = 0;
		desc.type = vx::gl::TextureType::Texture_2D;

		auto sid = s_objectManager->createTexture("volumetricTexture", desc);
		auto tex = s_objectManager->getTexture(sid);
		tex->setFilter(vx::gl::TextureFilter::LINEAR, vx::gl::TextureFilter::LINEAR);

		auto handle = tex->getTextureHandle();
		tex->makeTextureResident();

		auto uniformTextureBuffer = s_objectManager->getBuffer("UniformTextureBuffer");
		auto mappedBuffer = uniformTextureBuffer->map<Gpu::UniformTextureBufferBlock>(vx::gl::Map::Write_Only);
		mappedBuffer->u_volumetricTexture = handle;
	}

	void VolumetricLightRenderer::createFbo()
	{
		auto fboSid = s_objectManager->createFramebuffer("volumetricFbo");
		auto fbo = s_objectManager->getFramebuffer(fboSid);
		auto colorTexture = s_objectManager->getTexture("volumetricTexture");

		fbo->attachTexture(vx::gl::Attachment::Color0, *colorTexture, 0);
		fbo->drawBuffer(vx::gl::Attachment::Color0);
	}

	bool VolumetricLightRenderer::initialize(vx::StackAllocator* scratchAllocator, const void* p)
	{
		if (!s_shaderManager->loadPipeline(vx::FileHandle("volume.pipe"), "volume.pipe", scratchAllocator))
			return false;

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

		createTexture();
		createFbo();

		return true;
	}

	void VolumetricLightRenderer::shutdown()
	{

	}

	void VolumetricLightRenderer::getCommandList(CommandList* cmdList)
	{
		auto fbo = s_objectManager->getFramebuffer("volumetricFbo");
		auto vao = s_objectManager->getVertexArray("emptyVao");
		auto pipeline = s_shaderManager->getPipeline("volume.pipe");

		StateDescription stateDesc
		{
			fbo->getId(),
			vao->getId(),
			pipeline->getId(),
			0,
			0,
			true, 
			true,
			false,
			false,
			{1, 1, 1, 1},
			0
		};

		State state;
		state.set(stateDesc);

		Segment segment;
		segment.setState(state);

		auto halfRes = s_settings->m_resolution / 2u;

		ViewportCommand viewportCmd;
		viewportCmd.set(vx::uint2(0), halfRes);

		ClearCommand clearCmd;
		clearCmd.set(vx::gl::ClearBufferBit_Color);

		BlendEquationCommand blendEquCmd;
		blendEquCmd.set(GL_FUNC_ADD);

		BlendFuncCommand blendFuncCmd;
		blendFuncCmd.set(GL_ONE, GL_ONE);

		DrawArraysCommand drawCmd;
		drawCmd.set(GL_POINTS, 0, 1);

		GpuProfilePushCommand profilePushCmd;
		profilePushCmd.set(s_gpuProfiler, "volumetric");

		GpuProfilePopCommand profilePopCmd;
		profilePopCmd.set(s_gpuProfiler);

		segment.pushCommand(profilePushCmd);
		segment.pushCommand(viewportCmd);
		segment.pushCommand(clearCmd);
		segment.pushCommand(blendEquCmd);
		segment.pushCommand(blendFuncCmd);
		segment.pushCommand(drawCmd);
		segment.pushCommand(profilePopCmd);

		cmdList->pushSegment(segment, "drawVolumetric");
	}

	void VolumetricLightRenderer::clearData()
	{

	}

	void VolumetricLightRenderer::bindBuffers()
	{
		auto volumetricFogBuffer = s_objectManager->getBuffer("volumetricFogBuffer");

		//glBindBufferBase(GL_UNIFORM_BUFFER, 11, volumetricFogBuffer->getId());
	}
}