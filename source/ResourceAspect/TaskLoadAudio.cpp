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

#include "TaskLoadAudio.h"
#include <vxResourceAspect/ResourceManager.h>
#include <vxEngineLib/AudioFile.h>
#include <vxEngineLib/memcpy.h>
#include <Audioclient.h>

namespace TaskLoadAudioCpp
{
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

	bool loadWavFromMemory(const u8* data, AudioFileDesc* desc, ResourceManager<AudioFile>* audioDataManager)
	{
		u8 id[4]; //four bytes to hold 'RIFF' 

		data = vx::read(id, data);
		if (id[0] != 'R' ||
			id[1] != 'I' ||
			id[2] != 'F' ||
			id[3] != 'F')
		{
			return false;
		}

		u32 size = 0;
		data = vx::read(size, data);

		data = vx::read(id, data);
		if (id[0] != 'W' ||
			id[1] != 'A' ||
			id[2] != 'V' ||
			id[3] != 'E')
		{
			return false;
		}

		WaveFormatData formatData;
		data = vx::read(formatData, data);

		for (;;)
		{
			u8 tmp[4];
			data = vx::read(tmp, data);

			if (tmp[0] == 'd' &&
				tmp[1] == 'a' &&
				tmp[2] == 't' &&
				tmp[3] == 'a')
				break;
		}

		u32 dataSize = 0;
		data = vx::read(dataSize, data);

		std::unique_lock<std::mutex> lock;
		auto allocator = audioDataManager->lockDataAllocator(&lock);
		auto dataPtr = allocator->allocate<u8[]>(dataSize, 4);
		lock.unlock();

		if (dataPtr.get() == nullptr)
			return false;

		data = vx::read(dataPtr.get(), data, dataSize);

		auto type = AudioFileType::Invalid;
		if (formatData.format_tag == WAVE_FORMAT_IEEE_FLOAT)
		{
			type = AudioFileType::WAV;
		}
		else if (formatData.format_tag == WAVE_FORMAT_FLAC)
		{
			type = AudioFileType::FLAC;
		}

		desc->m_data = std::move(dataPtr);
		desc->m_type = type;
		desc->m_size = dataSize;
		desc->m_bytesPerSample = formatData.bits_per_sample / 8;
		desc->m_channels = formatData.channels;
		desc->m_samplesPerSec = formatData.sample_rate;

		return true;
	}
}

TaskLoadAudio::TaskLoadAudio(TaskLoadAudioDesc &&desc)
	:TaskLoadFile(std::move(desc.m_fileNameWithPath), desc.audioDataManager->getScratchAllocator(), desc.audioDataManager->getScratchAllocatorMutex(), std::move(desc.evt)),
	m_audioDataManager(desc.audioDataManager),
	m_sid(desc.m_sid),
	m_fileName(desc.m_fileName)
{

}

TaskLoadAudio::~TaskLoadAudio()
{

}

TaskReturnType TaskLoadAudio::runImpl()
{
	auto ptr = m_audioDataManager->find(m_sid);
	if (ptr != nullptr)
	{
		return TaskReturnType::Success;
	}

	managed_ptr<u8[]> data;
	u32 fileSize = 0;
	if (!loadFromFile(&data, &fileSize))
	{
		return TaskReturnType::Failure;
	}

	AudioFileDesc desc;
	if (!TaskLoadAudioCpp::loadWavFromMemory(data.get(), &desc, m_audioDataManager))
	{
		return TaskReturnType::Failure;
	}

	ptr = m_audioDataManager->insertEntry(m_sid, m_fileName.c_str(), std::move(desc));
	if (ptr == nullptr)
	{
		return TaskReturnType::Failure;
	}

	return TaskReturnType::Success;
}