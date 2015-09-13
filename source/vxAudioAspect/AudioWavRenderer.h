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
		WavFile* m_wavFile;
		WavFormat m_format;

	public:
		WavRenderer();
		WavRenderer(const WavRenderer&) = delete;
		WavRenderer(WavRenderer &&rhs);
		~WavRenderer();

		void setFile(WavFile* wavFile, const WavFormat &format)
		{
			m_wavFile = wavFile;
			m_format = format;
		}

		u32 readBuffer(u8* buffer, u32 frameCount) override
		{
			u32 readFrames = 0;
			auto bytesPerSample = m_format.m_bytesPerSample;
			if (bytesPerSample == 2)
			{
				if (m_dstBytes == 2)
				{
					readFrames = m_wavFile->loadDataShort(frameCount, m_format.m_channels, (s16*)buffer, m_dstChannels);
				}
				else if (m_dstBytes == 4)
				{
					readFrames = m_wavFile->loadDataShortToFloat(frameCount, m_format.m_channels, (float*)buffer, m_dstChannels);
				}
			}
			else if (bytesPerSample == 4)
			{
				readFrames = m_wavFile->loadDataFloat(frameCount, m_format.m_channels, (float*)buffer, m_dstChannels);
			}

			return readFrames;
		}

		void update() override
		{

		}

		u32 eof() const
		{
			return m_wavFile->eof();
		}
	};
}