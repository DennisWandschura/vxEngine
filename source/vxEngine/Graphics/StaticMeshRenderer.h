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
#pragma once

#include "Renderer.h"
#include <vxLib/gl/VertexArray.h>
#include <vxLib/gl/Buffer.h>

namespace Graphics
{
	class StaticMeshRenderer : public Renderer
	{
		static const U32 s_maxMeshInstanceCount{ 255u };
		static const U32 s_maxVertices = 100000;
		static const U32 s_maxIndices = 150000;

		vx::gl::Buffer m_indirectCmdBuffer;
		vx::gl::Buffer m_drawCountBuffer;
		vx::gl::VertexArray m_vao;
		vx::gl::Buffer m_vbo;
		vx::gl::Buffer m_idVbo;
		vx::gl::Buffer m_ibo;

		void createIndirectCmdBuffer();
		void createIbo();
		void createDrawIdVbo();
		void createVaoVbo();
		void bindBuffersToVao();

		Segment createSegmentGBuffer();

	public:
		void initialize() override;

		void update() override;

		void getSegments(std::vector<Segment>* segments) override;
	};
}