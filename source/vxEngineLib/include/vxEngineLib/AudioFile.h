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

#include <vxlib/types.h>
#include <vxenginelib/managed_ptr.h>

enum class AudioFileType : u32{ WAV, FLAC, Invalid };

struct AudioFileDesc
{
	managed_ptr<u8[]> m_data;
	u32 m_size;
	AudioFileType m_type;
	u32 m_samplesPerSec;
	u16 m_channels;
	u16 m_bytesPerSample;
};

class AudioFile
{
	managed_ptr<u8[]> m_data;
	u32 m_size;
	AudioFileType m_type;
	u32 m_samplesPerSec;
	u16 m_channels;
	u16 m_bytesPerSample;

public:
	AudioFile();
	AudioFile(const AudioFile&) = delete;
	AudioFile(AudioFile &&rhs);
	explicit AudioFile(AudioFileDesc &&desc);
	~AudioFile();

	AudioFile& operator=(const AudioFile&) = delete;
	AudioFile& operator=(AudioFile &&rhs);

	void swap(AudioFile &rhs);

	const u8* getData() const { return m_data.get(); }
	u32 getSize() const { return m_size; }
	AudioFileType getType() const { return m_type; }
	u32 getSamplesPerSec() const { return m_samplesPerSec; }
	u32 getChannels() const { return m_channels; }
	u32 getBytesPerSample() const { return m_bytesPerSample; }
};