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

namespace WavFileCpp
{
	template<typename SOURCE, typename DEST>
	struct ConvertData;

	template<typename T>
	struct ConvertData<T, T>
	{
		void operator()(T* dst, const T* src)
		{
			*dst = *src;
		}
	};

	template<>
	struct ConvertData<s16, float>
	{
		void operator()(float* dst, const s16* src)
		{
			const float cvt = 1.0f / 0x7fff;

			float tmp = *src;

			*dst = tmp * cvt;
		}
	};

	template<typename SRC_TYPE, typename DST_TYPE>
	u32 writeData(const u8* buffer, u32* head, u32 srcChannels, u32 dataSize, DST_TYPE* dst, u32 frameCount, u32 dstChannels)
	{
		const u32 srcChannelSize = srcChannels * sizeof(SRC_TYPE);
		u32 sizeInBytes = frameCount * srcChannelSize;

		u32 newHead = *head + sizeInBytes;
		newHead = std::min(newHead, dataSize);

		u32 readBytes = newHead - *head;
		u32 readFrames = readBytes / srcChannelSize;

		auto src = (SRC_TYPE*)(buffer + *head);

		auto remainingChannels = dstChannels - srcChannels;
		auto remainingDstSize = remainingChannels * sizeof(DST_TYPE);

		u32 index = 0;
		for (u32 i = 0; i < readFrames; ++i)
		{
			for (u32 chn = 0; chn < srcChannels; ++chn)
			{
				ConvertData<SRC_TYPE, DST_TYPE>()(dst++, src++);
			}

			memset(dst, 0, remainingDstSize);
			dst += remainingChannels;
		}

		*head += readBytes;

		return readFrames;
	}
}

WavFile::WavFile()
	:m_data(nullptr),
	m_dataSize(0),
	m_head(0)
{
}

WavFile::~WavFile()
{
	if (m_data)
	{
		delete[]m_data;
		m_data = nullptr;
		m_dataSize = 0;
	}
}

void WavFile::create(u8* data, u32 dataSize)
{
	m_data = data;
	m_dataSize = dataSize;
	m_head = 0;
}

u32 WavFile::loadDataFloat(u32 bufferFrameCount, u32 srcChannels, float* pData, u32 dstChannels)
{
	return WavFileCpp::writeData<float, float>(m_data, &m_head, srcChannels, m_dataSize, pData, bufferFrameCount, dstChannels);
}

u32 WavFile::loadDataShort(u32 bufferFrameCount, u32 srcChannels, s16* pData, u32 dstChannels)
{
	return WavFileCpp::writeData<s16, s16>(m_data, &m_head, srcChannels, m_dataSize, pData, bufferFrameCount, dstChannels);
}

u32 WavFile::loadDataShortToFloat(u32 bufferFrameCount, u32 srcChannels, float* pData, u16 dstChannels)
{
	return WavFileCpp::writeData<s16, float>(m_data, &m_head, srcChannels, m_dataSize, pData, bufferFrameCount, dstChannels);
}

struct WaveHeader
{
	u8 riff[4];
	u32 fileSize;
	u8 wave[4];
};

struct WaveFormatData
{
	u8 fmt[4];
	u32 formatSize;
	u16 format_tag;
	u16 channels;
	u32 sample_rate;
	u32 bytesPerSec;
	u16 block_align;
	u16 bits_per_sample;
};

struct WaveDataHeader
{
	u8 data[4];
	u32 dataSize;
};

namespace Audio
{
	bool loadWavFile(const char* name, WavFile* wavFile, WavFormat* format)
	{
		FILE* f = nullptr;
		f = fopen(name, "rb");
		if (f == nullptr)
		{
			return false;
		}

		u8 id[4]; //four bytes to hold 'RIFF' 
		u32 size; //32 bit value to hold file size 
		u32 formatSize = 0;

		fread(id, sizeof(u8), 4, f);
		if (id[0] != 'R' ||
			id[1] != 'I' ||
			id[2] != 'F' ||
			id[3] != 'F')
		{
			fclose(f);
			return false;
		}

		fread(&size, sizeof(u32), 1, f);

		fread(id, 1, 4, f);
		if (id[0] != 'W' ||
			id[1] != 'A' ||
			id[2] != 'V' ||
			id[3] != 'E')
		{
			fclose(f);
			return false;
		}

		WaveFormatData formatData;
		fread(&formatData, sizeof(WaveFormatData), 1, f);

		while (true)
		{
			u8 data[4];
			fread(data, 1, 4, f);

			if (data[0] == 'd' &&
				data[1] == 'a' &&
				data[2] == 't' &&
				data[3] == 'a')
				break;
		}

		u32 dataSize = 0;
		fread(&dataSize, sizeof(u32), 1, f);

		//WaveDataHeader dataHeader;
		//fread(&dataHeader, sizeof(WaveDataHeader), 1, f); //read in 'data' 

		auto buffer = new u8[dataSize];
		auto read = fread(buffer, sizeof(u8), dataSize, f);

		if (read != dataSize)
		{
			fclose(f);
			return false;
		}

		wavFile->create(buffer, dataSize);
		format->m_channels = formatData.channels;
		format->m_bytesPerSample = formatData.bits_per_sample / 8;

		fclose(f);

		return true;
	}
}