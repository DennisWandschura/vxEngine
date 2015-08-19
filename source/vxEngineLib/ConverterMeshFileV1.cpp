#include "ConverterMeshFileV1.h"
#include <vxEngineLib/MeshFile.h>
#include <vxEngineLib/ArrayAllocator.h>
#include <vxEngineLib/memcpy.h>
#include <memory>
#include <vxLib/util/CityHash.h>

namespace Converter
{
	const u8* MeshFileV1::loadFromMemory(const u8 *ptr, u32 size, ArrayAllocator* allocator, vx::MeshFile* meshFile)
	{
		u32 meshDataSize = 0;
		auto p = meshFile->m_mesh.loadFromMemoryDataSize(ptr, &meshDataSize);

		meshFile->m_meshData = allocator->allocate<u8[]>(meshDataSize, 4);
		if (meshFile->m_meshData.get() == nullptr)
		{
			return nullptr;
		}

		p = meshFile->m_mesh.loadFromMemoryData(p, meshFile->m_meshData.get(), meshDataSize);
		p = vx::read(meshFile->m_physxDataSize, p);
		p = vx::read(meshFile->m_physxMeshType, p);

		meshFile->m_physxData = allocator->allocate<u8[]>(meshFile->m_physxDataSize, 4);
		if (meshFile->m_physxData.get() == nullptr)
		{
			meshFile->m_physxDataSize = 0;
			return nullptr;
		}

		p = vx::read(meshFile->m_physxData.get(), p, meshFile->m_physxDataSize);

		VX_ASSERT((p - ptr) <= size);

		return p;
	}

	u64 MeshFileV1::getCrc(const vx::MeshFile &meshFile)
	{
		auto vertexCount = meshFile.m_mesh.getVertexCount();
		auto indexCount = meshFile.m_mesh.getIndexCount();
		auto vertices = meshFile.m_mesh.getVertices();
		auto indices = meshFile.m_mesh.getIndices();

		if (vertices == nullptr ||
			indices == nullptr)
		{
			printf("Error creating crc !\n");
			return 0;
		}

		auto meshVertexSize = sizeof(vx::MeshVertex) * vertexCount;
		auto meshIndexSize = sizeof(u32) * indexCount;
		auto meshSize = sizeof(u32) * 2 + meshVertexSize + meshIndexSize;
		auto totalSize = meshSize + sizeof(u32) + meshFile.m_physxDataSize;
		totalSize += sizeof(u32);

		auto ptr = std::make_unique<u8[]>(totalSize);
		auto p = ptr.get();

		::memcpy(p, &vertexCount, sizeof(u32));
		p += sizeof(u32);

		::memcpy(p, &indexCount, sizeof(u32));
		p += sizeof(u32);

		::memcpy(p, (u8*)vertices, meshVertexSize);
		p += meshVertexSize;

		::memcpy(p, (u8*)indices, meshIndexSize);
		p += meshIndexSize;

		::memcpy(p, &meshFile.m_physxDataSize, sizeof(u32));
		p += sizeof(u32);

		::memcpy(p, &meshFile.m_physxMeshType, sizeof(PhsyxMeshType));
		p += sizeof(PhsyxMeshType);

		::memcpy(p, meshFile.m_physxData.get(), meshFile.m_physxDataSize);
		p += meshFile.m_physxDataSize;

		auto writtenSize = p - ptr.get();
		if (writtenSize != totalSize)
		{
			printf("Error creating crc !\n");
			return 0;
		}

		return CityHash64((char*)ptr.get(), totalSize);
	}
}