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
	U32 m_isPlaying{0};
	ALuint m_source{0};		// audio source
	ALenum m_format{0};		// internal format

	void check();                 // checks OpenAL error state
	void empty();
	bool stream(ALuint buffer, U8* pBuffer, U32 bufferSize);   // reloads a buffer      

public:
	ogg_stream() = default;
	~ogg_stream();

	bool open(const char *path); // obtain a handle to the file
	void close();

	void play(U8* pBuffer, U32 bufferSize);        // play the Ogg stream
	bool isPlaying();         // check if the source is playing
	bool update(U8* pBuffer, U32 bufferSize); // update the stream if necessary
	void stop();

	//! from 0.0f to 1.0f
	void setVolume(float volume);
};
#endif