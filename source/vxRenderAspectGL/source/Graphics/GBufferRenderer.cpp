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

#include "vxRenderAspect/Graphics/GBufferRenderer.h"
#include <vxEngineLib/EngineConfig.h>
#include <vxRenderAspect/Graphics/Segment.h>
#include "vxRenderAspect/Graphics/Commands.h"
#include "vxRenderAspect/gl/ObjectManager.h"
#include <vxGL/ShaderManager.h>
#include <vxGL/ProgramPipeline.h>
#include <vxGL/Buffer.h>
#include "vxRenderAspect/Graphics/CommandList.h"
#include "vxRenderAspect/GpuStructs.h"
#include "vxRenderAspect/gl/BufferBindingManager.h"
#include <vxLib/File/FileHandle.h>

namespace Graphics
{
	GBufferRenderer::GBufferRenderer()
	{

	}

	GBufferRenderer::~GBufferRenderer()
	{

	}

	void GBufferRenderer::createTextures()
	{
		auto resolution = s_settings->m_resolution;
		vx::gl::TextureDescription desc;
		desc.type = vx::gl::TextureType::Texture_2D;
		desc.size = vx::ushort3(resolution.x, resolution.y, 1);
		desc.miplevels = 1;
		desc.sparse = 0;

		desc.format = vx::gl::TextureFormat::RGB8;
		s_objectManager->createTexture("gbufferAlbedoSlice", desc, true);

		desc.format = vx::gl::TextureFormat::RGBA16F;
		s_objectManager->createTexture("gbufferNormalSlice", desc, true);

		desc.format = vx::gl::TextureFormat::RGBA8;
		s_objectManager->createTexture("gbufferSurfaceSlice", desc, true);

		desc.format = vx::gl::TextureFormat::RGBA16F;
		s_objectManager->createTexture("gbufferTangentSlice", desc, true);

		s_objectManager->createTexture("gbufferBitangentSlice", desc, true);

		desc.format = vx::gl::TextureFormat::DEPTH32;
		desc.type = vx::gl::TextureType::Texture_2D;
		desc.size = vx::ushort3(resolution.x, resolution.y, 1);
		s_objectManager->createTexture("gbufferDepthSlice", desc, true);
	}

	void GBufferRenderer::initialize(vx::StackAllocator* scratchAllocator, const void*)
	{
		createTextures();

		auto tex0 = s_objectManager->getTexture("gbufferAlbedoSlice");
		auto tex1 = s_objectManager->getTexture("gbufferNormalSlice");
		auto tex2 = s_objectManager->getTexture("gbufferSurfaceSlice");
		auto tex3 = s_objectManager->getTexture("gbufferTangentSlice");
		auto tex4 = s_objectManager->getTexture("gbufferBitangentSlice");
		auto depth = s_objectManager->getTexture("gbufferDepthSlice");

		s_objectManager->createFramebuffer("gbufferFB");
		auto fbo = s_objectManager->getFramebuffer("gbufferFB");
		fbo->attachTexture(vx::gl::Attachment::Color0, *tex0, 0);
		fbo->attachTexture(vx::gl::Attachment::Color1, *tex1, 0);
		fbo->attachTexture(vx::gl::Attachment::Color2, *tex2, 0);
		fbo->attachTexture(vx::gl::Attachment::Color3, *tex3, 0);
		fbo->attachTexture(vx::gl::Attachment::Color4, *tex4, 0);
		fbo->attachTexture(vx::gl::Attachment::Depth, *depth, 0);

		vx::gl::Attachment buffers[] = {vx::gl::Attachment::Color0, vx::gl::Attachment::Color1,vx::gl::Attachment::Color2,vx::gl::Attachment::Color3, vx::gl::Attachment::Color4 };
		const auto bufferCount = sizeof(buffers) / sizeof(vx::gl::Attachment);
		fbo->drawBuffers(buffers, bufferCount);

		UniformGBufferBlock block;
		block.u_albedoSlice = tex0->getTextureHandle();
		block.u_normalSlice = tex1->getTextureHandle();
		block.u_surfaceSlice = tex2->getTextureHandle();
		block.u_tangentSlice = tex3->getTextureHandle();
		block.u_bitangentSlice = tex4->getTextureHandle();
		block.u_depthSlice = depth->getTextureHandle();

		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		desc.size = sizeof(UniformGBufferBlock);
		desc.immutable = 1;
		desc.pData = &block;

		s_objectManager->createBuffer("UniformGBufferBuffer", desc);

		s_shaderManager->loadPipeline(vx::FileHandle("drawMeshToGBuffer.pipe"), "drawMeshToGBuffer.pipe", scratchAllocator);
	}

	void GBufferRenderer::shutdown()
	{

	}

	void GBufferRenderer::update()
	{

	}

	void GBufferRenderer::getCommandList(CommandList* cmdList)
	{
		auto fbo = s_objectManager->getFramebuffer("gbufferFB");
		auto pipeline = s_shaderManager->getPipeline("drawMeshToGBuffer.pipe");
		auto cmdbuffer = s_objectManager->getBuffer("meshCmdBuffer");
		auto vao = s_objectManager->getVertexArray("meshVao");
		auto meshParamBuffer = s_objectManager->getBuffer("meshParamBuffer");

		StateDescription stateDesc = { fbo->getId(), vao->getId(), pipeline->getId(), cmdbuffer->getId(), meshParamBuffer->getId(), true, false, false };
		State state;
		state.set(stateDesc);

		//GpuProfilePushCommand gpuPushCmd;
		//gpuPushCmd.set(s_gpuProfiler, "gbuffer");
		//GpuProfilePopCommand gpuPopCmd;
		//gpuPopCmd.set(s_gpuProfiler);

		//CpuProfilePushCommand cpuPushCmd;
		//cpuPushCmd.set("gbuffer");
		//CpuProfilePopCommand cpuPopCmd;

		ViewportCommand viewportCmd;
		viewportCmd.set(vx::uint2(), s_settings->m_resolution);

		ClearColorCommand clearColorCmd;
		clearColorCmd.set(vx::float4(0, 0, 0, 0));

		ClearCommand clearCmd;
		clearCmd.set(vx::gl::ClearBufferBit_Color | vx::gl::ClearBufferBit_Depth);

		MultiDrawElementsIndirectCountCommand drawCmd;
		drawCmd.set((u32)vx::gl::PrimitveType::Triangles, (u32)vx::gl::DataType::Unsigned_Int, s_settings->m_rendererSettings.m_maxMeshInstances);

		Segment segmentCreateGBuffer;
		segmentCreateGBuffer.setState(state);
		//segmentCreateGBuffer.pushCommand(cpuPushCmd);
		//segmentCreateGBuffer.pushCommand(gpuPushCmd);
		segmentCreateGBuffer.pushCommand(viewportCmd);
		segmentCreateGBuffer.pushCommand(clearColorCmd);
		segmentCreateGBuffer.pushCommand(clearCmd);
		segmentCreateGBuffer.pushCommand(drawCmd);
		//segmentCreateGBuffer.pushCommand(gpuPopCmd);
		//segmentCreateGBuffer.pushCommand(cpuPopCmd);

		cmdList->clear();
		cmdList->pushSegment(segmentCreateGBuffer, "segmentCreateGBuffer");
	}

	void GBufferRenderer::clearData()
	{

	}

	void GBufferRenderer::bindBuffers()
	{
		auto buffer = s_objectManager->getBuffer("UniformGBufferBuffer");

		gl::BufferBindingManager::bindBaseUniform(11, buffer->getId());
	}
}