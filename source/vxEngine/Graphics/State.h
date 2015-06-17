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

#include <vxLib/types.h>
#include <string>

namespace Graphics
{
	struct StateDescription
	{
		u32 fbo{0};
		u32 vao{0};
		u32 pipeline{0};
		u32 indirectBuffer{0};
		u32 paramBuffer{0};
		bool depthState{true};
		bool blendState{false};
		bool polygonOffsetFillState{false};
	};

	class State
	{
		u32 m_fbo;
		u32 m_vao;
		u32 m_pipeline;
		u32 m_indirectBuffer;
		u32 m_paramBuffer;

		u8 m_blendState;
		u8 m_depthTestState;
		u8 m_polygonOffsetFillState;
		u8 m_padding;

	public:
		State();
		~State();

		void set(const StateDescription &desc);

		void update() const;

		bool isValid() const;
	};
}