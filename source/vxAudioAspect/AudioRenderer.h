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
		u16 dstChannels; 
		u16 dstBytes;
		IAudioRenderClient* audioRenderClient;
		IAudioClient* audioClient;
		f32 waitTime;
	};

	class Renderer
	{
		typedef u32(*LoadDataFunction)(void*, u32, u32, u8*, AudioChannels, f32, const __m128*, const __m128*);

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
		AudioChannels m_dstChannels;	// 1
		u16 m_dstBytes;					// 2

	public:
		Renderer() :m_renderClient(nullptr), m_audioClient(nullptr), m_fp(nullptr), m_id(nullptr), m_waitTime(0), m_accum(0), m_bufferFrames(0), m_dstChannels(), m_dstBytes(0) {}
		explicit Renderer(RendererDesc &&desc);
		Renderer(const Renderer &rhs) = delete;
		Renderer(Renderer &&rhs);

		virtual ~Renderer();

		Renderer& operator=(const Renderer &rhs) = delete;
		Renderer& operator=(Renderer &&rhs);

		void destroy();

		virtual u32 readBuffer(u8* buffer, u32 frameCount, f32 intensity, const __m128* direction, const __m128* rotation) = 0;
		virtual void update() = 0;

		void start(const __m128 &listenerPosition, const __m128 &listenerDirection);
		void play(f32 dt, const __m128 &listenerPosition, const __m128 &listenerRotation);
		void stop();

		virtual u32 eof() const = 0;

		void setPosition(const vx::float3 &position);
	};
}