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