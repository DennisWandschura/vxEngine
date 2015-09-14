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

#include <vxEngineLib/AudioFile.h>
#include <vxEngineLib/ArrayAllocator.h>

AudioFile::AudioFile()
	:m_data(),
	m_size(0),
	m_type(),
	m_samplesPerSec(0),
	m_channels(0),
	m_bytesPerSample(0)
{

}

AudioFile::AudioFile(AudioFile &&rhs)
	:m_data(std::move(rhs.m_data)),
	m_size(rhs.m_size),
	m_type(rhs.m_type),
	m_samplesPerSec(rhs.m_samplesPerSec),
	m_channels(rhs.m_channels),
	m_bytesPerSample(rhs.m_bytesPerSample)
{

}

AudioFile::AudioFile(AudioFileDesc &&desc)
	:m_data(std::move(desc.m_data)),
	m_size(desc.m_size),
	m_type(desc.m_type),
	m_samplesPerSec(desc.m_samplesPerSec),
	m_channels(desc.m_channels),
	m_bytesPerSample(desc.m_bytesPerSample)
{

}

AudioFile::~AudioFile()
{

}

AudioFile& AudioFile::operator=(AudioFile &&rhs)
{
	if (this != &rhs)
	{
		swap(rhs);
	}
	return *this;
}

void AudioFile::swap(AudioFile &rhs)
{
	m_data.swap(rhs.m_data);
	std::swap(m_size, rhs.m_size);
	std::swap(m_type, rhs.m_type);
	std::swap(m_samplesPerSec, rhs.m_samplesPerSec);
	std::swap(m_channels, rhs.m_channels);
	std::swap(m_bytesPerSample, rhs.m_bytesPerSample);
}