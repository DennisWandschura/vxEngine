#include <vxEngineLib/ActorFile.h>
#include <vxLib/File/File.h>
#include <vxEngineLib/memcpy.h>
#include <vxLib/util/CityHash.h>

ActorFile::ActorFile(u32 version)
	:vx::Serializable(version)
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
}

const u8* ActorFile::loadFromMemory(const u8 *ptr, u32 size, vx::Allocator* allocator)
{
	ptr = vx::read(m_mesh, ptr);
	ptr = vx::read(m_material, ptr);

	return ptr;
}

u64 ActorFile::getCrc() const
{
	auto version = getVersion();

	u64 crc = 0;
	if (version == 0)
	{
		const auto bufferSize = sizeof(m_mesh) + sizeof(m_material);
		u8 buffer[bufferSize];
		memset(buffer,0, bufferSize);

		auto p = vx::write(buffer, m_mesh);
		vx::write(p, m_material);

		crc = CityHash64((char*)buffer, bufferSize);
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