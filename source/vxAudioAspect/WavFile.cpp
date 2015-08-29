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

/*#include <vxAudio/WavFile.h>
#include <fstream>

struct WavHeader
{
	char sGroupID[4];
	unsigned int dwFileLength;
	char sRiffType[4];
};

struct WavFormatChunk
{
	char sGroupID[4];
	unsigned int dwChunkSize;
	unsigned short wFormatTag;
	unsigned short wChannels;
	unsigned int dwSamplesPerSec;
	unsigned int dwAvgBytesPerSec;
	unsigned short wBlockAlign;
	unsigned short dwBitsPerSample;
};

struct WavChunk
{
	char sGroupID[4];
	unsigned int dwChunkSize;
};

WavFile::WavFile()
	:m_data(),
	m_size(0)
{

}

WavFile::~WavFile()
{

}

bool WavFile::loadFromFile(const char* file)
{
	std::ifstream inFile(file, std::ios::binary);
	if (!inFile.is_open())
		return false;

	inFile.seekg(0, std::ifstream::end);
	auto sz = inFile.tellg();
	inFile.seekg(0, std::ifstream::beg);

	auto memory = std::make_unique<unsigned char[]>(sz);

	inFile.read((char*)memory.get(), sz);
	inFile.close();

	WavHeader* header = (WavHeader*)memory.get();
	WavFormatChunk* formatChunk = (WavFormatChunk*)(header + 1);

	WavChunk* dataChuck = nullptr;
	unsigned char* ptr = (unsigned char*)(formatChunk + 1);
	auto ptrEnd = memory.get() + sz;
	while (true)
	{
		WavChunk* chunk = (WavChunk*)(ptr);
		if (chunk->sGroupID[0] == 'd' &&
			chunk->sGroupID[1] == 'a' &&
			chunk->sGroupID[2] == 't'&&
			chunk->sGroupID[3] == 'a')
		{
			dataChuck = chunk;
			break;
		}

		if (ptr >= ptrEnd)
			break;

		ptr += chunk->dwChunkSize + sizeof(WavChunk);
	}

	if (dataChuck == nullptr)
		return false;

	ptr += sizeof(WavChunk);
	m_data = std::make_unique<unsigned char[]>(dataChuck->dwChunkSize);
	memcpy(m_data.get(), ptr, dataChuck->dwChunkSize);
	m_size = dataChuck->dwChunkSize;

	return true;
}*/