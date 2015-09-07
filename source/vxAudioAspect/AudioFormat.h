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

namespace Wav
{
	struct Header
	{
		u8 riff[4];
		u32 fileSize;
		u8 wave[4];

		bool isValid() const
		{
			auto cmp0 = riff[0] == 'R' && riff[1] == 'I' && riff[2] == 'F' && riff[3] == 'F';
			auto cmp1 = wave[0] == 'W' && wave[1] == 'A' && wave[2] == 'V' && wave[3] == 'E';

			return cmp0 && cmp1;
		}
	};

	struct FormatHeader
	{
		u8 fmt[4];
		u32 formatSize;
		u16 formatTag;
		u16 channels;
		u32 sampleRate;
		u32 bytesPerSec;
		u16 blockAlign;
		u16 bitsPerSample;
	};

	struct DataHeader
	{
		u8 data[4];
		u32 dataSize;
	};
};