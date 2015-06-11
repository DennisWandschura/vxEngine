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

#include "include/vxEngineLib/MeshFile.h"
#include <vxLib/Allocator/Allocator.h>
#include <vxLib/util/CityHash.h>
#include <vxLib/memory.h>
#include <vxLib/File/File.h>

namespace vx
{
	MeshFile::MeshFile()
		:m_mesh(),
		m_physxData(nullptr),
		m_physxDataSize(0)
	{

	}

	MeshFile::MeshFile(MeshFile &&rhs)
		:m_mesh(std::move(rhs.m_mesh)),
		m_physxData(rhs.m_physxData),
		m_physxDataSize(rhs.m_physxDataSize)
	{
		rhs.m_physxData = nullptr;
		rhs.m_physxDataSize = 0;
	}

	MeshFile::MeshFile(vx::Mesh &&mesh, const u8* physxData, u32 physxDataSize)
		:m_mesh(std::move(mesh)),
		m_physxData(physxData),
		m_physxDataSize(physxDataSize)
	{
	}

	MeshFile::~MeshFile()
	{
		m_physxDataSize = 0;
		m_physxData = nullptr;
	}

	MeshFile& MeshFile::operator = (MeshFile &&rhs)
	{
		if (this != &rhs)
		{
			m_mesh.swap(rhs.m_mesh);
			std::swap(m_physxData, rhs.m_physxData);
			std::swap(m_physxDataSize, rhs.m_physxDataSize);
		}
		return *this;
	}

	const u8* MeshFile::loadFromMemory(const u8 *ptr, u32 size, u32 version, vx::Allocator* allocator)
	{
		if (version != getVersion())
		{
			// convert old version to new version
		}

		auto p = m_mesh.loadFromMemory(ptr, allocator);
		if (p == nullptr)
			return nullptr;

		memcpy(&m_physxDataSize, p, sizeof(u32));
		p += sizeof(u32);

		m_physxData = allocator->allocate(m_physxDataSize, 4);
		if (m_physxData == nullptr)
		{
			m_physxDataSize = 0;
			return nullptr;
		}

		memcpy((void*)m_physxData, p, m_physxDataSize);

		return (p + m_physxDataSize);
	}

	void MeshFile::saveToFile(vx::File* file) const
	{
		if (!m_mesh.saveToFile(file))
			return;

		file->write(m_physxDataSize);
		file->write(m_physxData, m_physxDataSize);
	}

	u64 MeshFile::getCrc() const
	{
		auto vertexCount = m_mesh.getVertexCount();
		auto indexCount = m_mesh.getIndexCount();
		auto vertices = m_mesh.getVertices();
		auto indices = m_mesh.getIndices();

		if (vertices == nullptr ||
			indices == nullptr)
		{
			printf("Error creating crc !\n");
			return 0;
		}

		auto meshVertexSize = sizeof(vx::MeshVertex) * vertexCount;
		auto meshIndexSize = sizeof(u32) * indexCount;
		auto meshSize = sizeof(u32) * 2 + meshVertexSize + meshIndexSize;
		auto totalSize = meshSize + sizeof(u32) + m_physxDataSize;

		auto ptr = vx::make_unique<u8[]>(totalSize);
		auto p = ptr.get();

		memcpy(p, &vertexCount, sizeof(u32));
		p += sizeof(u32);

		memcpy(p, &indexCount, sizeof(u32));
		p += sizeof(u32);

		memcpy(p, (u8*)vertices, meshVertexSize);
		p += meshVertexSize;

		memcpy(p, (u8*)indices, meshIndexSize);
		p += meshIndexSize;

		memcpy(p, &m_physxDataSize, sizeof(u32));
		p += sizeof(u32);

		memcpy(p, m_physxData, m_physxDataSize);
		p += m_physxDataSize;

		auto writtenSize = p - ptr.get();
		if (writtenSize != totalSize)
		{
			printf("Error creating crc !\n");
			return 0;
		}

		return CityHash64((char*)ptr.get(), totalSize);
	}

	u32 MeshFile::getVersion() const
	{
		return 0;
	}
}