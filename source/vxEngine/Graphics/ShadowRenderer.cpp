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

#include "ShadowRenderer.h"
#include "../gl/ObjectManager.h"
#include "../RenderSettings.h"
#include <vxLib/gl/gl.h>
#include <string>
#include "Segment.h"
#include "Commands.h"
#include <vxLib/gl/ShaderManager.h>
#include <vxLib/gl/ProgramPipeline.h>
#include <vxLib/gl/Buffer.h>
#include "../GpuStructs.h"
#include "../gl/BufferBindingManager.h"
#include "Commands/ProgramUniformCommand.h"

namespace Graphics
{
	ShadowRenderer::ShadowRenderer()
		:m_textureCount(0)
	{

	}

	ShadowRenderer::~ShadowRenderer()
	{

	}

	void ShadowRenderer::createShadowTextureBuffer()
	{
		UniformShadowTextureBufferBlock data;
		memset(&data, 0, sizeof(UniformShadowTextureBufferBlock));

		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		desc.flags = vx::gl::BufferStorageFlags::Write;
		desc.immutable = 1;
		desc.pData = &data;
		desc.size = sizeof(UniformShadowTextureBufferBlock);

		auto sid = s_objectManager->createBuffer("uniformShadowTextureBuffer", desc);
		VX_ASSERT(sid != 0);
	}

	void ShadowRenderer::createShadowTextures()
	{
		m_textureCount = s_settings->m_maxActiveLights;
		auto textureResolution = s_settings->m_shadowmapResolution;

		auto shadowTexBuffer = s_objectManager->getBuffer("uniformShadowTextureBuffer");

		m_shadowTextureIds = std::make_unique<U32[]>(m_textureCount);

		vx::gl::TextureDescription desc;
		desc.format = vx::gl::TextureFormat::DEPTH32F;
		desc.type = vx::gl::TextureType::Texture_Cubemap;
		desc.size = vx::ushort3(textureResolution, textureResolution, 6);
		desc.miplevels = 1;
		desc.sparse = 0;

		const char textureName[] = "shadowTexture";
		const auto textureNameSize = sizeof(textureName);
		char nameBuffer[24];
		memcpy(nameBuffer, textureName, textureNameSize);

		auto mappedBuffer = shadowTexBuffer->map<UniformShadowTextureBufferBlock>(vx::gl::Map::Write_Only);

		for (U32 i = 0; i < m_textureCount; ++i)
		{
			sprintf(nameBuffer + textureNameSize - 1, "%u", i);

			auto sid = s_objectManager->createTexture(nameBuffer, desc);
			VX_ASSERT(sid != 0);

			auto texture = s_objectManager->getTexture(sid);

			m_shadowTextureIds[i] = texture->getId();

			texture->setFilter(vx::gl::TextureFilter::LINEAR, vx::gl::TextureFilter::LINEAR);
			texture->setWrapMode3D(vx::gl::TextureWrapMode::CLAMP_TO_EDGE, vx::gl::TextureWrapMode::CLAMP_TO_EDGE, vx::gl::TextureWrapMode::CLAMP_TO_EDGE);

			glTextureParameteri(texture->getId(), GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTextureParameteri(texture->getId(), GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
			//float ones[] = { 0, 0, 0, 0 };
			//glTextureParameterfv(m_pColdData->m_shadowTexture.getId(), GL_TEXTURE_BORDER_COLOR, ones);

			texture->makeTextureResident();

			mappedBuffer->u_shadowTextures[i] = texture->getTextureHandle();
		}
	}

	void ShadowRenderer::createFramebuffer()
	{
		auto sid = s_objectManager->createFramebuffer("shadowFbo");
		auto fbo = s_objectManager->getFramebuffer(sid);

		glNamedFramebufferDrawBuffer(fbo->getId(), GL_NONE);
	}

	void ShadowRenderer::initialize()
	{
		createShadowTextureBuffer();
		createShadowTextures();
		createFramebuffer();
	}

	void ShadowRenderer::update()
	{

	}

	void ShadowRenderer::getSegments(std::vector<Segment>* segments)
	{
		auto maxMeshInstances = s_settings->m_maxMeshInstances;
		auto resolution = s_settings->m_shadowmapResolution;

		auto fbo = s_objectManager->getFramebuffer("shadowFbo");
		auto vao = s_objectManager->getVertexArray("meshVao");
		auto cmdBuffer = s_objectManager->getBuffer("meshCmdBuffer");
		auto paramBuffer = s_objectManager->getBuffer("meshParamBuffer");
		auto pipeline = s_shaderManager->getPipeline("shadow.pipe");
		auto gsShader = pipeline->getGeometryShader();

		Graphics::StateDescription stateDesc;
		stateDesc.fbo = fbo->getId();
		stateDesc.vao = vao->getId();
		stateDesc.blendState = false;
		stateDesc.depthState = true;
		stateDesc.indirectBuffer = cmdBuffer->getId();
		stateDesc.paramBuffer = paramBuffer->getId();
		stateDesc.pipeline = pipeline->getId();
		stateDesc.polygonOffsetFillState = true;

		Graphics::State state;
		state.set(stateDesc);

		Graphics::ViewportCommand viewportCmd;
		viewportCmd.set(vx::uint2(0), vx::uint2(resolution, resolution));

		Graphics::PolygonOffsetCommand polyCmd;
		polyCmd.set(2.5f, 10.0f);

		Graphics::MultiDrawElementsIndirectCountCommand drawCmd;
		drawCmd.set(GL_TRIANGLES, GL_UNSIGNED_INT, maxMeshInstances);

		Graphics::Segment segmentCreateShadowmap;
		segmentCreateShadowmap.setState(state);

		segmentCreateShadowmap.pushCommand(viewportCmd);
		segmentCreateShadowmap.pushCommand(polyCmd);

		Graphics::ProgramUniformCommand uniformCmd;
		uniformCmd.setUInt(gsShader, 0, 1);

		auto maxLightCount = s_settings->m_maxActiveLights;
		for (U32 i = 0; i < maxLightCount; ++i)
		{

			Graphics::ProgramUniformData<U32> uniformData;
			uniformData.set(i);

			Graphics::FramebufferTextureCommand fbTexCmd;
			fbTexCmd.set(fbo->getId(), GL_DEPTH_ATTACHMENT, m_shadowTextureIds[i], 0);

			Graphics::ClearCommand clearCmd;
			clearCmd.set(GL_DEPTH_BUFFER_BIT);

			segmentCreateShadowmap.pushCommand(fbTexCmd);
			segmentCreateShadowmap.pushCommand(clearCmd);
			segmentCreateShadowmap.pushCommand(uniformCmd, uniformData);
			segmentCreateShadowmap.pushCommand(drawCmd);
		}

		segments->push_back(segmentCreateShadowmap);
	}

	void ShadowRenderer::clearData()
	{
	}

	void ShadowRenderer::bindBuffers()
	{
		auto shadowTexBuffer = s_objectManager->getBuffer("uniformShadowTextureBuffer");
		gl::BufferBindingManager::bindBaseUniform(9, shadowTexBuffer->getId());
	}

	const U32* ShadowRenderer::getTextureIds() const
	{
		return m_shadowTextureIds.get();
	}
}