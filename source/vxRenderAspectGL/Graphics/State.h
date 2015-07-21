#pragma once
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

#include <vxLib/math/Vector.h>
#include <string>

namespace Graphics
{
	struct StateDescription
	{
		u32 m_fbo;
		u32 m_vao;
		u32 m_pipeline;
		u32 m_indirectBuffer;
		u32 m_paramBuffer;
		bool m_depthState;
		bool m_blendState;
		bool m_polygonOffsetFillState;
		bool m_cullface;
		vx::uchar4 m_colorMask;
		u8 m_depthMask;

		StateDescription() :m_fbo(0), m_vao(0), m_pipeline(0), m_indirectBuffer(0), m_paramBuffer(0), m_depthState(true), m_blendState(false), m_polygonOffsetFillState(false), m_cullface(true), m_colorMask(1, 1, 1, 1), m_depthMask(1){}
		StateDescription(u32 fbo, u32 vao, u32 pipeline, u32 cmd, u32 param, bool depth, bool blend, bool polyOffsetState, bool cullFace, const vx::uchar4 &colorMask, u8 depthMask)
			:m_fbo(fbo), m_vao(vao), m_pipeline(pipeline), m_indirectBuffer(cmd), m_paramBuffer(param), m_depthState(depth), m_blendState(blend), m_polygonOffsetFillState(polyOffsetState), m_cullface(cullFace), m_colorMask(colorMask), m_depthMask(depthMask) {}
	};

	class State
	{
		enum GlState : u8;

		u32 m_fbo;
		u32 m_vao;
		u32 m_pipeline;
		u32 m_indirectBuffer;
		u32 m_paramBuffer;

		u8 m_state;
		/*u8 m_blendState;
		u8 m_depthTestState;
		u8 m_polygonOffsetFillState;
		u8 m_cullFace;*/

		u8 m_colorMask;

	public:
		State();
		~State();

		void set(const StateDescription &desc);

		void update() const;

		bool isValid() const;
	};
}