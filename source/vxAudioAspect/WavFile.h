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

#include "AudioEnums.h"
#include <vxLib/math/Vector.h>

class WavFile
{
	const u8* m_data;
	u32 m_dataSize;
	u32 m_head;

public:
	WavFile();
	WavFile(const WavFile&);
	WavFile(WavFile &&rhs);
	WavFile(const u8* data, u32 dataSize);
	~WavFile();

	WavFile& operator=(const WavFile &rhs);
	WavFile& operator=(WavFile &&rhs);

	u32 loadDataFloat(u32 bufferFrameCount, u32 srcChannels, u8* pData, AudioChannels dstChannels, f32 intensity, const __m128* direction, const __m128* rotation);

	u32 loadDataShort(u32 bufferFrameCount, u32 srcChannels, u8* pData, AudioChannels dstChannels, f32 intensity, const __m128* direction, const __m128* rotation);

	u32 loadDataShortToFloat(u32 bufferFrameCount, u32 srcChannels, u8* pData, AudioChannels dstChannels, f32 intensity, const __m128* direction, const __m128* rotation);

	u32 eof() const
	{
		return (m_dataSize == m_head);
	}
};