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

struct IAudioRenderClient;
struct IAudioClient;

#include <vxLib/math/Vector.h>
#include "AudioEnums.h"

namespace Audio
{
	struct RendererDesc
	{
		u32 bufferFrames;
		IAudioRenderClient* audioRenderClient;
		IAudioClient* audioClient;
		f32 waitTime;
	};

	class Renderer
	{
		typedef u32(*LoadDataFunction)(void* p, u32 bufferFrameCount, u32 srcChannels, u8* pData, u8 dstChannelCount, f32 intensity, const __m128(&directions)[8], const __m128 &directionToListener);

		IAudioRenderClient* m_renderClient; // 8
		IAudioClient* m_audioClient;		// 8
		f32 m_waitTime;						// 4
		f32 m_accum;						// 4
		u32 m_bufferFrames;					// 4
		u32 m_padding;						// 4

	protected:
		__m128 m_position;				// 16
		LoadDataFunction m_fp;			// 8
		u32* m_id;						// 8

	public:
		Renderer() :m_renderClient(nullptr), m_audioClient(nullptr), m_fp(nullptr), m_id(nullptr), m_waitTime(0), m_accum(0), m_bufferFrames(0) {}
		explicit Renderer(RendererDesc &&desc);
		Renderer(const Renderer &rhs) = delete;
		Renderer(Renderer &&rhs);

		virtual ~Renderer();

		Renderer& operator=(const Renderer &rhs) = delete;
		Renderer& operator=(Renderer &&rhs);

		void destroy();

		virtual u32 readBuffer(u8* buffer, u32 frameCount, u8 dstChannnelCount, f32 intensity, const __m128(&directions)[8], const __m128 &directionToListener) = 0;
		virtual void update() = 0;

		void start(u8 dstChannelCount, const __m128(&directions)[8], const __m128 &listenerPosition);
		void play(u8 dstChannelCount, f32 dt, const __m128(&directions)[8], const __m128 &listenerPosition);
		void stop();

		virtual u32 eof() const = 0;

		void setPosition(const vx::float3 &position);
	};
}