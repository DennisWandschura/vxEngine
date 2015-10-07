#include <vxEngineLib/ActorFile.h>
#include <vxLib/File/File.h>
#include "ConverterActorFileV0.h"
#include <cstring>
#include "ConverterActorFileV1.h"

ActorFile::ActorFile(u32 version)
	:vx::Serializable(version),
	m_fovRad(1.1519173383712769f)
{
	m_mesh[0] = '\0';
	m_material[0] = '\0';
}

ActorFile::~ActorFile()
{

}

void ActorFile::saveToFile(vx::File* f) const
{
	f->write(m_mesh);
	f->write(m_material);
	f->write(m_fovRad);
}

const u8* ActorFile::loadFromMemory(const u8 *ptr, u32 size, vx::Allocator* allocator)
{
	auto version = getVersion();

	switch (version)
	{
	case 0:
		ptr = Converter::ActorFileV0::loadFromMemory(ptr, size, nullptr, this);
		break;
	case 1:
		ptr = Converter::ActorFileV1::loadFromMemory(ptr, size, nullptr, this);
		break;
	default:
		break;
	}

	return ptr;
}

u64 ActorFile::getCrc() const
{
	auto version = getVersion();

	u64 crc = 0;
	switch (version)
	{
	case 0:
		crc = Converter::ActorFileV0::getCrc(*this);
		break;
	case 1:
		crc = Converter::ActorFileV1::getCrc(*this);
		break;
	default:
		break;
	}

	return crc;
}

void ActorFile::setMesh(const char(&mesh)[32])
{
	memcpy(m_mesh, mesh, 32);
}

void ActorFile::setMaterial(const char(&material)[32])
{
	memcpy(m_material, material, 32);
}