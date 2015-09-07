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

#include <vxLib/types.h>

class WavFile
{
	u8* m_data;
	u32 m_head;
	u32 m_dataSize;

public:
	WavFile();
	WavFile(const WavFile &) = delete;
	WavFile(WavFile &&rhs);
	~WavFile();

	WavFile& operator=(const WavFile&) = delete;
	WavFile& operator=(WavFile &&rhs);

	void create(u8* data, u32 size)
	{
		m_data = data;
		m_dataSize = size;
	}

	u32 readDataFloat(u32 frameCount, u32 srcChannels, f32* dst, u32 dstChannels);

	u32 readDataFloat22(u32 frameCount, f32* dst);
	u32 readDataFloat28(u32 frameCount, f32* dst);

	const u8* getData() const { return m_data; }
	u32 getSize() const { return m_dataSize; }
};