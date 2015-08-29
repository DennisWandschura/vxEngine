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
/*#include <vxAudio/ogg_stream.h>

//#define BUFFER_SIZE (4096 * 8)

ogg_stream::~ogg_stream()
{
	if (m_source != 0)
	{
		alSourceStop(m_source);
		empty();

		alDeleteSources(1, &m_source);
		check();
	}

	if (m_buffers[0] != 0)
	{
		alDeleteBuffers(BUFFERS, m_buffers);
		check();
	}

	ov_clear(&m_vf);
}

bool ogg_stream::open(const char *path)
{
	int code = ov_fopen(path, &m_vf);
	if (code != 0)
	{
		printf("could not open file '%s', code %d\n", path, code);
		return false;
	}

	m_pVorbisInfo = ov_info(&m_vf, -1);

	if (m_pVorbisInfo->channels == 1)
		m_format = AL_FORMAT_MONO16;
	else
		m_format = AL_FORMAT_STEREO16;

	//m_pBuffers = new ALuint[BUFFERS];
	alGenBuffers(BUFFERS, m_buffers);
	check();

	alGenSources(1, &m_source);
	check();

	alSource3f(m_source, AL_POSITION, 0, 0.0, 0.0);
	alSource3f(m_source, AL_VELOCITY, 0.0f, 0.0, 0.0);
	alSource3f(m_source, AL_DIRECTION, 0.0f, 0.0, 0.0);
	alSourcef(m_source, AL_ROLLOFF_FACTOR, 1.0f);
	alSourcei(m_source, AL_SOURCE_RELATIVE, AL_FALSE);

	return true;
}

void ogg_stream::close()
{

}

void ogg_stream::play(u8* pBuffer, u32 bufferSize)
{
	m_isPlaying = 1;

	// check if source is already playing
	if (isPlaying())
		return;

	// write data into buffers
	for (unsigned int i = 0; i < BUFFERS; ++i)
	{
		if (!stream(m_buffers[i], pBuffer, bufferSize))
			return;
	}

	alSourceQueueBuffers(m_source, BUFFERS, m_buffers);

	// start playing
	alSourcePlay(m_source);
}

bool ogg_stream::update(u8* pBuffer, u32 bufferSize)
{
	int processed;
	bool active = true;

	alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed);

	while (processed--)
	{
		ALuint buffer;

		alSourceUnqueueBuffers(m_source, 1, &buffer);
		check();

		active = stream(buffer, pBuffer, bufferSize);

		if (active)
		{
			alSourceQueueBuffers(m_source, 1, &buffer);
			check();
		}
	}

	auto cmp = (m_isPlaying != 0) & active & !isPlaying();

	// if we still are supposed to be playing, but the source stopped, restart it
	if (cmp != 0)
	{
		alSourcePlay(m_source);
		//active = true;
	}

	return active;
}

bool ogg_stream::stream(ALuint buffer, u8* pBuffer, u32 bufferSize)
{
	//char data[BUFFER_SIZE];
	int size = 0;

	while (size < (s32)bufferSize)
	{
		int section;
		int result = ov_read(&m_vf, (s8*)pBuffer + size, bufferSize - size, 0, 2, 1, &section);

		if (result > 0)
		{
			size += result;
		}
		else
		{
			VX_ASSERT(result >= 0);
			//if (result < 0)
			//	return false;
			//	throw std::exception();
			//throw oggString(result);
			break;
		}
	}

	if (size == 0)
		return false;

	alBufferData(buffer, m_format, pBuffer, size, m_pVorbisInfo->rate);
	check();

	return true;
}

void ogg_stream::empty()
{
	int queued;

	alGetSourcei(m_source, AL_BUFFERS_QUEUED, &queued);

	while (queued--)
	{
		ALuint buffer;

		alSourceUnqueueBuffers(m_source, 1, &buffer);
		check();
	}
}

bool ogg_stream::isPlaying()
{
	ALenum state;
	alGetSourcei(m_source, AL_SOURCE_STATE, &state);

	return (state == AL_PLAYING);
}

void ogg_stream::setVolume(float volume)
{
	alSourcef(m_source, AL_GAIN, volume);
}

void ogg_stream::check()
{
	int error = alGetError();

	//if (error != AL_NO_ERROR)
	//	throw std::exception("OpenAL error was raised.");
	VX_ASSERT(error == AL_NO_ERROR);
}

void ogg_stream::stop()
{
	m_isPlaying = 0;
	alSourceStop(m_source);
}*/