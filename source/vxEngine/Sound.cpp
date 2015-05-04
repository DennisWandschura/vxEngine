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
#include "Sound.h"

Sound::Sound()
	:m_sourceId(0),
	m_position()
{
	alGenSources(1, &m_sourceId);
}

Sound::Sound(F32 x, F32 y, F32 z)
	:m_sourceId(0),
	m_position(x, y, z)
{
	alGenSources(1, &m_sourceId);

	alSource3f(m_sourceId, AL_POSITION, m_position.x, m_position.y, m_position.z);
}

Sound::~Sound()
{
	if (m_sourceId != 0)
	{
		alDeleteSources(1, &m_sourceId);
		m_sourceId = 0;
	}
}

void Sound::setPosition(const vx::float3 &position)
{
	m_position = position;
	alSource3f(m_sourceId, AL_POSITION, m_position.x, m_position.y, m_position.z);
}
#endif