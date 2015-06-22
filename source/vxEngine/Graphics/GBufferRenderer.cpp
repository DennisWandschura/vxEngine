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

#include "GBufferRenderer.h"

namespace Graphics
{
	GBufferRenderer::GBufferRenderer()
	{

	}

	GBufferRenderer::~GBufferRenderer()
	{

	}

	void GBufferRenderer::initialize(const void* p)
	{

	}

	void GBufferRenderer::shutdown()
	{

	}

	void GBufferRenderer::update()
	{

	}

	void GBufferRenderer::getCommandList(CommandList* cmdList)
	{
		/*
		vx::gl::StateManager::setClearColor(0, 0, 0, 0);
		vx::gl::StateManager::setViewport(0, 0, m_resolution.x, m_resolution.y);
		vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);

		vx::gl::StateManager::bindFrameBuffer(m_gbufferFB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		vx::gl::StateManager::bindBuffer(vx::gl::BufferType::Parameter_Buffer, *meshParamBuffer);

		auto pPipeline = m_shaderManager.getPipeline("drawMeshToGBuffer.pipe");
		vx::gl::StateManager::bindPipeline(pPipeline->getId());
		vx::gl::StateManager::bindVertexArray(*meshVao);
		vx::gl::StateManager::bindBuffer(vx::gl::BufferType::Draw_Indirect_Buffer, *meshCmdBuffer);
		glMultiDrawElementsIndirectCountARB(GL_TRIANGLES, GL_UNSIGNED_INT, 0, 0, 150, sizeof(vx::gl::DrawElementsIndirectCommand));
		*/
	}

	void GBufferRenderer::clearData()
	{

	}

	void GBufferRenderer::bindBuffers()
	{

	}
}