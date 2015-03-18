#if !defined(_VX_NOAUDIO) && !defined(_VX_EDITOR)
#pragma once

#include <AL/al.h>
#include <vxLib/math/Vector.h>

class Sound
{
	U32 m_sourceId;
	vx::float3 m_position;

public:
	Sound();

	Sound(F32 x, F32 y, F32 z);

	~Sound();

	void setPosition(const vx::float3 &position);
};
#endif