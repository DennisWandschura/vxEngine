#pragma once

#include <vxLib/Graphics/Mesh.h>
#include "Serializable.h"

class MeshFile : public Serializable
{
	vx::Mesh m_mesh;

public:
	const U8* loadFromMemory(const U8 *ptr, U32 version);

	U64 getCrc() const;

	U32 getVersion() const;
};