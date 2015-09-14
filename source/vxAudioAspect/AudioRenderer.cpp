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
	Renderer::Renderer(RendererDesc &&desc)
		:m_renderClient(desc.audioRenderClient),
		m_audioClient(desc.audioClient),
		m_waitTime(desc.waitTime), 
		m_accum(0), 
		m_bufferFrames(desc.bufferFrames), 
		m_dstChannels(desc.dstChannels),
		m_dstBytes(desc.dstBytes) 
	{
		desc.audioClient = nullptr;
		desc.audioRenderClient = nullptr;
	}

	Renderer::Renderer(Renderer &&rhs)
		:m_renderClient(rhs.m_renderClient),
		m_audioClient(rhs.m_audioClient),
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

	Renderer& Renderer::operator=(Renderer &&rhs)
	{
		if (this != &rhs)
		{
			std::swap(m_renderClient, rhs.m_renderClient);
			std::swap(m_audioClient, rhs.m_audioClient);
			std::swap(m_waitTime, rhs.m_waitTime);
			std::swap(m_accum, rhs.m_accum);
			std::swap(m_bufferFrames, rhs.m_bufferFrames);
			std::swap(m_dstChannels, rhs.m_dstChannels);
			std::swap(m_dstBytes, rhs.m_dstBytes);
		}
		return *this;
	}

	void Renderer::startPlay()
	{
		BYTE *pData = nullptr;
		auto hr = m_renderClient->GetBuffer(m_bufferFrames, &pData);

		auto read_count = readBuffer(pData, m_bufferFrames);

		hr = m_renderClient->ReleaseBuffer((UINT32)read_count, 0);

		hr = m_audioClient->Start();
	}

	void Renderer::play(f32 dt)
	{
		m_accum += dt;

		if (m_accum >= m_waitTime)
		{
			u32 numFramesPadding = 0;
			auto hr = m_audioClient->GetCurrentPadding(&numFramesPadding);

			UINT32 numAvailableFrames = m_bufferFrames - numFramesPadding;

			u8* data = nullptr;
			hr = m_renderClient->GetBuffer(numAvailableFrames, &data);

			auto read_count = readBuffer(data, numAvailableFrames);

			hr = m_renderClient->ReleaseBuffer((UINT32)read_count, 0);

			m_accum -= m_waitTime;
		}
	}
}