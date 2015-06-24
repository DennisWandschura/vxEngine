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
#include "ScreenshotFactory.h"
#include <vxLib/stb_image_write.h>
#include <vxLib/memory.h>
#include <time.h>

namespace
{
	void writeScreenshotToFile(const vx::uint2 &resolution, vx::float4a *pData)
	{
		auto size = resolution.x * resolution.y * 3;
		std::unique_ptr<u8[]> pngData = vx::make_unique<u8[]>(size);

		const __m128 vToUINT8 = { 255.0f, 255.0f, 255.0f, 255.0f };

		for (u32 i = 0, j = 0; i < size; i += 3, ++j)
		{
			__m128 vTmp = _mm_load_ps(&pData[j].x);
			vTmp = _mm_mul_ps(vTmp, vToUINT8);

			_mm_store_ps(&pData[j].x, vTmp);

			pngData[i] = pData[j].x;
			pngData[i + 1] = pData[j].y;
			pngData[i + 2] = pData[j].z;
		}

		_aligned_free(pData);

		u8 *last_row = pngData.get() + (resolution.x * 3 * (resolution.y - 1));

		__time64_t long_time;
		_time64(&long_time);
		tm tm;
		localtime_s(&tm, &long_time);

		char nameBuffer[36];
		sprintf_s(nameBuffer, "screenshot %04i-%02i-%02i_%02i.%02i.%02i.png", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

		stbi_write_png(nameBuffer, resolution.x, resolution.y, 3, last_row, -3 * resolution.x);
	}
}

void ScreenshotFactory::writeScreenshotToFile(const vx::uint2 &resolution, vx::float4a* data)
{
//	std::async(::writeScreenshotToFile, resolution, data);
}