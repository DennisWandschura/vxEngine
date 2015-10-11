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

#include "WavFile.h"
#include <algorithm>
#include "WavFormat.h"
#include <DirectXMath.h>

namespace WavFileCpp
{
	template<typename SOURCE, typename DEST>
	struct ConvertData;

	template<typename T>
	struct ConvertData<T, T>;

	template<>
	struct ConvertData<s16, s16>
	{
		void operator()(s16* dst, const s16* src, f32 intensity)
		{
			const float cvt = 1.0f / 0x7fff;
			float tmp = (*src) * cvt * intensity;

			*dst = static_cast<s16>(tmp * 0x7fff);
		}
	};

	template<>
	struct ConvertData<f32, f32>
	{
		void operator()(f32* dst, const f32* src, f32 intensity)
		{
			*dst = (*src) * intensity;
		}
	};

	template<>
	struct ConvertData<s16, float>
	{
		void operator()(float* dst, const s16* src, f32 intensity)
		{
			const float cvt = 1.0f / 0x7fff;

			float tmp = *src;

			*dst = (tmp * cvt) * intensity;
		}
	};

	template<typename SRC_TYPE, typename DST_TYPE>
	u32 writeDataSrc1(const u8* buffer, u32* head, u32 srcChannels, u32 dataSize, DST_TYPE* dst, u32 frameCount,
		u8 dstChannelCount, f32 intensity, const __m128(&directions)[8], const __m128 &directionToListener)
	{
		const u32 srcChannelSize = srcChannels * sizeof(SRC_TYPE);
		u32 sizeInBytes = frameCount * srcChannelSize;

		u32 newHead = *head + sizeInBytes;
		newHead = std::min(newHead, dataSize);

		u32 readBytes = newHead - *head;
		u32 readFrames = readBytes / srcChannelSize;

		auto src = (SRC_TYPE*)(buffer + *head);

		const auto dstSize = sizeof(DST_TYPE) * dstChannelCount;

		const __m128 upDir = { 0, 1, 0, 0 };

		auto absDirectionToListenerY = vx::abs(_mm_and_ps(directionToListener, vx::g_VXMaskY));
		auto dpy = vx::dot3(absDirectionToListenerY, upDir);

		for (u32 i = 0; i < readFrames; ++i)
		{
			memset(dst, 0, dstSize);

			for (u32 chn = 0; chn < dstChannelCount; ++chn)
			{
				auto channelDir = directions[chn];

				auto dot = _mm_max_ps(vx::dot3(channelDir, directionToListener), vx::g_VXZero);
				f32 weight = 1.0f - dot.m128_f32[0];

				f32 directionalIntensity = dot.m128_f32[0] + dpy.m128_f32[0] * weight;

				ConvertData<SRC_TYPE, DST_TYPE>()(dst++, src, intensity * directionalIntensity);
			}
			++src;
		}

		*head += readBytes;

		return readFrames;
	}

	template<typename SRC_TYPE, typename DST_TYPE>
	u32 writeData(const u8* buffer, u32* head, u32 srcChannels, u32 dataSize, DST_TYPE* dst, u32 frameCount,
		u8 dstChannelCount, f32 intensity, const __m128(&directions)[8], const __m128 &directionToListener)
	{
		const u32 srcChannelSize = srcChannels * sizeof(SRC_TYPE);
		u32 sizeInBytes = frameCount * srcChannelSize;

		u32 newHead = *head + sizeInBytes;
		newHead = std::min(newHead, dataSize);

		u32 readBytes = newHead - *head;
		u32 readFrames = readBytes / srcChannelSize;

		auto src = (SRC_TYPE*)(buffer + *head);

		if (srcChannels == 2)
		{
			//const auto remainingChannels = 6;
			//const auto remainingDstSize = remainingChannels * sizeof(DST_TYPE);

			const auto dstSize = sizeof(DST_TYPE) * dstChannelCount;

			const __m128 upDir = { 0, 1, 0, 0 };

			auto absDirectionToListenerY = vx::abs(_mm_and_ps(directionToListener, vx::g_VXMaskY));
			auto dpy = vx::dot3(absDirectionToListenerY, upDir);

			//u32 index = 0;
			for (u32 i = 0; i < readFrames; ++i)
			{
				memset(dst, 0, dstSize);

				for (u32 chn = 0; chn < dstChannelCount; ++chn)
				{
					auto channelDir = directions[chn];

					auto dot = _mm_max_ps(vx::dot3(channelDir, directionToListener), vx::g_VXZero);
					f32 weight = 1.0f - dot.m128_f32[0];

					f32 directionalIntensity = dot.m128_f32[0] + dpy.m128_f32[0] * weight;

					ConvertData<SRC_TYPE, DST_TYPE>()(dst++, src, intensity * directionalIntensity);
				}
				src += 2;

				//memset(dst, 0, remainingDstSize);
				//dst += remainingChannels;

				/*for (u32 chn = 0; chn < srcChannels; ++chn)
				{
					ConvertData<SRC_TYPE, DST_TYPE>()(dst++, src++, intensity);
				}*/
			}

			*head += readBytes;
		}
		else
		{
			const auto dstSize = sizeof(DST_TYPE) * dstChannelCount;
			u32 index = 0;
			for (u32 i = 0; i < readFrames; ++i)
			{
				memset(dst, 0, dstSize);
				dst += dstChannelCount;

				*head += readBytes;
			}
		}

		return readFrames;
	}
}

WavFile::WavFile()
	:m_data(nullptr),
	m_dataSize(0),
	m_head(0)
{
}

WavFile::WavFile(const WavFile &rhs)
	:m_data(rhs.m_data),
	m_dataSize(rhs.m_dataSize),
	m_head(rhs.m_head)
{
}

WavFile::WavFile(WavFile &&rhs)
	:m_data(rhs.m_data),
	m_dataSize(rhs.m_dataSize),
	m_head(rhs.m_head)
{
	rhs.m_data = nullptr;
	rhs.m_dataSize = 0;
}

WavFile::WavFile(const u8* data, u32 dataSize)
	:m_data(data),
	m_dataSize(dataSize),
	m_head(0)
{
}

WavFile::~WavFile()
{
	m_data = nullptr;
	m_dataSize = 0;
}

WavFile& WavFile::operator=(const WavFile &rhs)
{
	if (this != &rhs)
	{
		m_data = rhs.m_data;
		m_dataSize = rhs.m_dataSize;
		m_head = rhs.m_head;
	}
	return *this;
}

WavFile& WavFile::operator=(WavFile &&rhs)
{
	if (this != &rhs)
	{
		std::swap(m_data, rhs.m_data);
		std::swap(m_dataSize, rhs.m_dataSize);
		std::swap(m_head, rhs.m_head);
	}
	return *this;
}

u32 WavFile::loadDataFloat(u32 bufferFrameCount, u32 srcChannels, u8* pData, u8 dstChannelCount, f32 intensity, const __m128(&directions)[8], const __m128 &directionToListener)
{
	return WavFileCpp::writeData<float, float>(m_data, &m_head, srcChannels, m_dataSize, (f32*)pData, bufferFrameCount, dstChannelCount, intensity, directions, directionToListener);
}

u32 WavFile::loadDataShort(u32 bufferFrameCount, u32 srcChannels, u8* pData, u8 dstChannelCount, f32 intensity, const __m128(&directions)[8], const __m128 &directionToListener)
{
	return WavFileCpp::writeData<s16, s16>(m_data, &m_head, srcChannels, m_dataSize, (s16*)pData, bufferFrameCount, dstChannelCount, intensity, directions, directionToListener);
}

u32 WavFile::loadDataShortToFloat(u32 bufferFrameCount, u32 srcChannels, u8* pData, u8 dstChannelCount, f32 intensity, const __m128(&directions)[8], const __m128 &directionToListener)
{
	return WavFileCpp::writeData<s16, float>(m_data, &m_head, srcChannels, m_dataSize, (f32*)pData, bufferFrameCount, dstChannelCount, intensity, directions, directionToListener);
}