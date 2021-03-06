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

#include "AudioRenderer.h"
#include "WavFile.h"
#include "WavFormat.h"

namespace Audio
{
	class WavRenderer : public Renderer
	{
		WavFile m_wavFile;
		WavFormat m_format;

	public:
		WavRenderer();
		WavRenderer(const WavRenderer &rhs) = delete;
		WavRenderer(WavRenderer &&rhs);
		explicit WavRenderer(RendererDesc &&desc);
		~WavRenderer();

		WavRenderer& operator=(const WavRenderer&) = delete;
		WavRenderer& operator=(WavRenderer &&rhs);

		void initialize(const WavFile &wavFile, const WavFormat &format, u32 dstBytesPerSample, const vx::float3 &position, u32* id);

		u32 readBuffer(u8* buffer, u32 frameCount, u8 dstChannnelCount, f32 intensity, const __m128(&directios)[8], const __m128 &directionToListener) override;

		void update() override
		{

		}

		u32 eof() const;
	};
}