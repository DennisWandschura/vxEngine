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

namespace WavFileCpp
{
	template<typename u32 SIZESRC, u32 SIZEDST>
	struct CopyArray;

	template<>
	struct CopyArray<2, 2>
	{
		template<typename T>
		static void copy(const T* src, T* dst)
		{
			dst[0] = src[0];
			dst[1] = src[1];
		}
	};

	template<>
	struct CopyArray<2, 8>
	{
		template<typename T>
		static void copy(const T* src, T* dst)
		{
			dst[0] = src[0];
			dst[1] = src[1];
			dst[2] = 0;
			dst[3] = 0;

			dst[4] = 0;
			dst[5] = 0;
			dst[6] = 0;
			dst[7] = 0;
		}
	};

	template<u32 SRCCHANNELS, u32 DSTCHANNELS>
	struct Reader
	{
		u32 static readShort(u32 frameCount, const u16* src, u16* dst, const u16* end, u32* head)
		{
			const u32 srcTotalSize = frameCount * SRCCHANNELS;

			auto newHead = src + srcTotalSize;
			newHead = std::min(newHead, end);

			auto readBytes = newHead - src;
			auto readFrames = readBytes / SRCCHANNELS;

			u32 index = 0;
			for (u32 i = 0; i < readFrames; ++i)
			{
				CopyArray<SRCCHANNELS, DSTCHANNELS>::copy(src, dst);

				src += SRCCHANNELS;
				dst += DSTCHANNELS;
			}

			*head += static_cast<u32>(readBytes);

			return static_cast<u32>(readFrames);
		}

		u32 static readFloat(u32 frameCount, const f32* src, f32* dst, const f32* end, u32* head)
		{
			const u32 srcTotalSize = frameCount * SRCCHANNELS;

			auto newHead = src + srcTotalSize;
			newHead = std::min(newHead, end);

			auto readBytes = newHead - src;
			auto readFrames = readBytes / SRCCHANNELS;

			u32 index = 0;
			for (u32 i = 0; i < readFrames; ++i)
			{
				CopyArray<SRCCHANNELS, DSTCHANNELS>::copy(src, dst);

				src += SRCCHANNELS;
				dst += DSTCHANNELS;
			}

			*head += static_cast<u32>(readBytes);

			return static_cast<u32>(readFrames);
		}
	};
}

WavFile::WavFile()
	:m_data(nullptr),
	m_head(0),
	m_dataSize(0)
{

}

WavFile::WavFile(WavFile &&rhs)
	:m_data(rhs.m_data),
	m_head(rhs.m_head),
	m_dataSize(rhs.m_dataSize)
{
	rhs.m_data = nullptr;
	m_head = 0;
	m_dataSize = 0;
}

WavFile::~WavFile()
{
	if (m_data)
	{
		delete[]m_data;
	}
}

WavFile& WavFile::operator=(WavFile &&rhs)
{
	if (this != &rhs)
	{
		std::swap(m_data, rhs.m_data);
		std::swap(m_head, rhs.m_head);
		std::swap(m_dataSize, rhs.m_dataSize);
	}
	return *this;
}

u32 WavFile::readDataFloat22(u32 frameCount, f32* dst)
{
	/*const u32 channels = 2;
	const u32 srcTotalSize = frameCount * channels * sizeof(f32);

	u32 newHead = m_head + srcTotalSize;
	newHead = std::min(newHead, m_dataSize);

	u32 readBytes = newHead - m_head;
	u32 readFrames = readBytes / channels;

	//
	auto read = readBytes / channels;
	auto ptr = (float*)(m_data + m_head);

	u32 index = 0;
	for (u32 i = 0; i < read; ++i)
	{
		dst[0] = ptr[0];
		dst[1] = ptr[0];

		ptr += channels;
		dst += channels;
	}

	m_head += readBytes;

	return readFrames;*/

	auto src = (float*)(m_data + m_head);
	auto end = (float*)(m_data + m_dataSize);

	return WavFileCpp::Reader<2, 2>::readFloat(frameCount, src, dst, end, &m_head);
}

u32 WavFile::readDataFloat28(u32 frameCount, f32* dst)
{
	const u32 dstChannels = 8;
	const u32 srcChannels = 2;
	const u32 srcTotalSize = frameCount * srcChannels * sizeof(f32);

	u32 newHead = m_head + srcTotalSize;
	newHead = std::min(newHead, m_dataSize);

	u32 readBytes = newHead - m_head;
	u32 readFrames = readBytes / srcChannels;

	//
	auto read = readBytes / srcChannels;
	auto ptr = (float*)(m_data + m_head);

	u32 index = 0;
	for (u32 i = 0; i < read; ++i)
	{
		dst[0] = ptr[0];
		dst[1] = ptr[0];
		dst[2] = 0;
		dst[3] = 0;

		dst[4] = 0;
		dst[5] = 0;
		dst[6] = 0;
		dst[7] = 0;

		ptr += srcChannels;
		dst += dstChannels;
	}

	m_head += readBytes;

	return readFrames;
}

u32 WavFile::readDataFloat(u32 frameCount, u32 srcChannels, f32* dst, u32 dstChannels)
{
	const u32 srcChannelBytes = srcChannels * sizeof(f32);
	u32 srcTotalSize = frameCount * srcChannelBytes;

	u32 newHead = m_head + srcTotalSize;
	newHead = std::min(newHead, m_dataSize);

	u32 readBytes = newHead - m_head;
	u32 readFrames = readBytes / srcChannels;

	//
	auto read = readBytes / srcChannels;
	auto ptr = (float*)(m_data + m_head);

	auto remainingChannels = dstChannels - srcChannels;

	u32 index = 0;
	for (u32 i = 0; i < read; ++i)
	{
		for (u32 chn = 0; chn < srcChannels; ++chn)
		{
			float value = ptr[index++];
			*dst = value;
			++dst;
		}

		for (u32 chn = 0; chn < remainingChannels; ++chn)
		{
			*dst = 0.0f;
			++dst;
		}
	}

	m_head += readBytes;

	return readFrames;
}