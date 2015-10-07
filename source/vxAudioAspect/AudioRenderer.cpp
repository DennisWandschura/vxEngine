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
#include <Audioclient.h>
#include <algorithm>

namespace Audio
{
	f32 idlFunction(f32 s, f32 dp)
	{
		const f32 speedOfSound = 343.0f;
		const f32 headRadius = 0.3f;
		const f32 beta = 2 * speedOfSound / headRadius;

		const f32 alpha = 1 + dp;

		return (alpha * s + beta) / (s + beta);
	}

	f32 idtFunction(f32 dp)
	{
		const f32 speedOfSound = 343.0f;
		const f32 headRadius = 0.3f;

		f32 result = 0.0f;
		if (dp < 0.5f)
		{
			result = headRadius / speedOfSound * (1.0f - dp);
		}
		else //if (vx::VX_PIDIV2 < dp && 
			//dp < vx::VX_PI)
		{
			result = headRadius / speedOfSound * (dp + 1.0f - vx::VX_PIDIV2);
		}
		return result;
	}

	Renderer::Renderer(RendererDesc &&desc)
		:m_renderClient(desc.audioRenderClient),
		m_audioClient(desc.audioClient),
		m_position(),
		m_fp(nullptr),
		m_id(nullptr),
		m_waitTime(desc.waitTime), 
		m_accum(0), 
		m_bufferFrames(desc.bufferFrames), 
		m_dstChannels(),
		m_dstBytes(desc.dstBytes) 
	{
		desc.audioClient = nullptr;
		desc.audioRenderClient = nullptr;

		m_dstChannels = AudioChannels::Channel2;
		if (desc.dstChannels == 6)
		{
			m_dstChannels = AudioChannels::Channel6;
		}
		else if (desc.dstChannels == 8)
		{
			m_dstChannels = AudioChannels::Channel8;
		}
	}

	Renderer::Renderer(Renderer &&rhs)
		:m_renderClient(rhs.m_renderClient),
		m_audioClient(rhs.m_audioClient),
		m_position(rhs.m_position),
		m_fp(rhs.m_fp),
		m_id(rhs.m_id),
		m_waitTime(rhs.m_waitTime),
		m_accum(rhs.m_accum),
		m_bufferFrames(rhs.m_bufferFrames),
		m_dstChannels(rhs.m_dstChannels),
		m_dstBytes(rhs.m_dstBytes)
	{
		rhs.m_renderClient = nullptr;
		rhs.m_audioClient = nullptr;
	}

	Renderer::~Renderer()
	{
		destroy();
	}

	Renderer& Renderer::operator=(Renderer &&rhs)
	{
		if (this != &rhs)
		{
			std::swap(m_renderClient, rhs.m_renderClient);
			std::swap(m_audioClient, rhs.m_audioClient);
			std::swap(m_position, rhs.m_position);
			std::swap(m_fp, rhs.m_fp);
			std::swap(m_id, rhs.m_id);
			std::swap(m_waitTime, rhs.m_waitTime);
			std::swap(m_accum, rhs.m_accum);
			std::swap(m_bufferFrames, rhs.m_bufferFrames);
			std::swap(m_dstChannels, rhs.m_dstChannels);
			std::swap(m_dstBytes, rhs.m_dstBytes);
		}
		return *this;
	}

	void Renderer::destroy()
	{
		if (m_renderClient)
		{
			m_renderClient->Release();
			m_renderClient = nullptr;
		}

		if (m_audioClient)
		{
			m_audioClient->Release();
			m_audioClient = nullptr;
		}
	}

	void Renderer::start(const __m128 &listenerPosition, const __m128 &listenerRotation)
	{
		BYTE *pData = nullptr;
		auto hr = m_renderClient->GetBuffer(m_bufferFrames, &pData);
		if (hr == 0)
		{
			//const f32 maxDist = 20.0f;
			//const f32 refDist = 5.0f;

			auto directionToListener =  _mm_sub_ps(listenerPosition, m_position);
			auto distance = vx::length3(directionToListener);
			directionToListener = _mm_div_ps(directionToListener, distance);

			auto intensity = 1.0f / (1.0f + distance.m128_f32[0] * distance.m128_f32[0]);

			auto read_count = readBuffer(pData, m_bufferFrames, intensity, &directionToListener, &listenerRotation);

			hr = m_renderClient->ReleaseBuffer((UINT32)read_count, 0);
		}

		hr = m_audioClient->Start();
	}

	void Renderer::play(f32 dt, const __m128 &listenerPosition, const __m128 &listenerRotation)
	{
		m_accum += dt;

		if (m_accum >= m_waitTime)
		{
			//const f32 maxDist = 20.0f;
			//const f32 refDist = 5.0f;

			auto directionToListener = _mm_sub_ps(listenerPosition, m_position);
			auto distance = vx::length3(directionToListener);
			directionToListener = _mm_div_ps(directionToListener, distance);

			//auto tmpdistance = vx::clamp(distance, refDist, maxDist);
			//auto intensity = refDist / (refDist + (tmpdistance - refDist));
			auto intensity = 1.0f / (1.0f + distance.m128_f32[0] * distance.m128_f32[0]);

			u32 numFramesPadding = 0;
			auto hr = m_audioClient->GetCurrentPadding(&numFramesPadding);

			UINT32 numAvailableFrames = m_bufferFrames - numFramesPadding;

			u8* data = nullptr;
			hr = m_renderClient->GetBuffer(numAvailableFrames, &data);

			auto read_count = readBuffer(data, numAvailableFrames, intensity, &directionToListener, &listenerRotation);

			hr = m_renderClient->ReleaseBuffer((UINT32)read_count, 0);

			m_accum -= m_waitTime;
		}
	}

	void Renderer::stop()
	{
		*m_id = 0xffffffff;
		m_audioClient->Stop();
	}

	void Renderer::setPosition(const vx::float3 &position)
	{
		m_position = vx::loadFloat3(position);
	}
}