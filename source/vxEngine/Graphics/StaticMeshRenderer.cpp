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
#include "StaticMeshRenderer.h"
#include "Segment.h"
#include "BufferFactory.h"
#include "../Vertex.h"
#include <vxLib/gl/ShaderManager.h>
#include <vxLib/gl/ProgramPipeline.h>
#include "Commands.h"
#include <vxLib/gl/gl.h>

namespace Graphics
{
	void StaticMeshRenderer::createIndirectCmdBuffer()
	{
		auto sizeInBytes = sizeof(vx::gl::DrawElementsIndirectCommand) * s_maxMeshInstanceCount;

		vx::gl::DrawElementsIndirectCommand cmd[s_maxMeshInstanceCount];
		memset(cmd, 0, sizeInBytes);

		m_indirectCmdBuffer = BufferFactory::createIndirectCmdBuffer(sizeInBytes, vx::gl::BufferStorageFlags::Write, (U8*)cmd);

		sizeInBytes = sizeof(U32);
		U32 count = 0;
		m_drawCountBuffer = BufferFactory::createParameterBuffer(sizeInBytes, vx::gl::BufferStorageFlags::Write, (U8*)&count);
	}

	void StaticMeshRenderer::createIbo()
	{
		auto sizeInBytes = sizeof(U32) * s_maxIndices;
		
		m_ibo = BufferFactory::createIndexBuffer(sizeInBytes, vx::gl::BufferStorageFlags::Write);
	}

	void StaticMeshRenderer::createDrawIdVbo()
	{
		auto sizeInBytes = sizeof(U32) * s_maxMeshInstanceCount;
		m_idVbo = BufferFactory::createVertexBuffer(sizeInBytes, vx::gl::BufferStorageFlags::Write);
	}

	void StaticMeshRenderer::createVaoVbo()
	{
		U32 attributeOffset = 0;
		VertexPNTUV::create(&m_vao, &m_vbo, s_maxVertices, 0, attributeOffset);
	}

	void StaticMeshRenderer::bindMeshDrawIdVboToVao()
	{
		m_vao.enableArrayAttrib(4);
		m_vao.arrayAttribFormatI(4, 1, vx::gl::DataType::Unsigned_Int, 0);
		m_vao.arrayAttribBinding(4, 1);
		m_vao.arrayBindingDivisor(1, 1);
		m_vao.bindVertexBuffer(m_idVbo, 1, 0, sizeof(U32));
	}

	void StaticMeshRenderer::bindBuffersToVao()
	{
		bindMeshDrawIdVboToVao();
		m_vao.bindIndexBuffer(m_ibo);
	}

	void StaticMeshRenderer::initialize()
	{
		createIndirectCmdBuffer();
		createIbo();
		createDrawIdVbo();
		createVaoVbo();
		bindBuffersToVao();
	}

	void StaticMeshRenderer::update()
	{

	}

	Segment StaticMeshRenderer::createSegmentGBuffer()
	{
		/*vx::gl::StateManager::setClearColor(0, 0, 0, 0);
		vx::gl::StateManager::setViewport(0, 0, m_resolution.x, m_resolution.y);
		vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);

		vx::gl::StateManager::bindFrameBuffer(m_gbufferFB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		auto pPipeline = m_shaderManager.getPipeline("create_gbuffer.pipe");
		vx::gl::StateManager::bindPipeline(pPipeline->getId());

		vx::gl::StateManager::bindVertexArray(vao);

		cmdBuffer.bind();
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, count, sizeof(vx::gl::DrawElementsIndirectCommand));*/

		auto pPipeline = s_shaderManager->getPipeline("create_gbuffer.pipe");

		State state;
		//state.set(, m_vao.getId(), pPipeline->getId(), m_indirectCmdBuffer.getId(), m_drawCountBuffer.getId());
		state.setBlendState(false);
		state.setDepthTest(true);

		ClearColorCommand clearColorCommand;
		clearColorCommand.set(vx::float4(0.0f));

		MultiDrawElementsIndirectCountCommand drawCmd;
		drawCmd.set(GL_TRIANGLES, GL_UNSIGNED_INT, s_maxMeshInstanceCount);

		Segment segment;
		segment.setState(state);
		segment.pushCommand(clearColorCommand);
		segment.pushCommand(drawCmd);

		return segment;
	}

	void StaticMeshRenderer::getSegments(std::vector<Segment>* segments)
	{

	}
}