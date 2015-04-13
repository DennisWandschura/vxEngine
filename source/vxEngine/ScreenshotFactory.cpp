#include "ScreenshotFactory.h"
#include <future>
#include <vxLib/stb_image_write.h>

namespace
{
	void writeScreenshotToFile(const vx::uint2 &resolution, vx::float4a *pData)
	{
		auto size = resolution.x * resolution.y * 3;
		std::unique_ptr<U8[]> pngData = std::make_unique<U8[]>(size);

		const __m128 vToUINT8 = { 255.0f, 255.0f, 255.0f, 255.0f };

		for (U32 i = 0, j = 0; i < size; i += 3, ++j)
		{
			__m128 vTmp = _mm_load_ps(&pData[j].x);
			vTmp = _mm_mul_ps(vTmp, vToUINT8);

			_mm_store_ps(&pData[j].x, vTmp);

			pngData[i] = pData[j].x;
			pngData[i + 1] = pData[j].y;
			pngData[i + 2] = pData[j].z;
		}

		_aligned_free(pData);

		U8 *last_row = pngData.get() + (resolution.x * 3 * (resolution.y - 1));

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
	std::async(::writeScreenshotToFile, resolution, data);
}