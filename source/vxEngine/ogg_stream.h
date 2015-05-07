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
#if !defined(_VX_NOAUDIO) && !defined(_VX_EDITOR)
#pragma once

struct OggVorbis_File;

#include <vorbis/vorbisfile.h>
#include <al/al.h>
#include <vxLib/types.h>

#define BUFFERS 3

class ogg_stream
{
	OggVorbis_File m_vf{};
	ALuint m_buffers[BUFFERS]; // front and back buffers
	vorbis_info *m_pVorbisInfo{nullptr};    // some formatting data
	u32 m_isPlaying{0};
	ALuint m_source{0};		// audio source
	ALenum m_format{0};		// internal format

	void check();                 // checks OpenAL error state
	void empty();
	bool stream(ALuint buffer, u8* pBuffer, u32 bufferSize);   // reloads a buffer      

public:
	ogg_stream() = default;
	~ogg_stream();

	bool open(const char *path); // obtain a handle to the file
	void close();

	void play(u8* pBuffer, u32 bufferSize);        // play the Ogg stream
	bool isPlaying();         // check if the source is playing
	bool update(u8* pBuffer, u32 bufferSize); // update the stream if necessary
	void stop();

	//! from 0.0f to 1.0f
	void setVolume(float volume);
};
#endif