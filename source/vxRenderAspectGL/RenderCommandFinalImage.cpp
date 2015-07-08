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
#include "RenderCommandFinalImage.h"
#include <vxGL/gl.h>
#include <vxGL/StateManager.h>

void RenderCommandFinalImage::render(u32 count)
{
	vx::gl::StateManager::bindFrameBuffer(m_frameBuffer);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);

	vx::gl::StateManager::bindPipeline(m_pipeline);
	vx::gl::StateManager::setViewport(0, 0, m_resolution.x, m_resolution.y);
	vx::gl::StateManager::bindVertexArray(m_vao);

	glDrawArrays(GL_POINTS, 0, count);
}