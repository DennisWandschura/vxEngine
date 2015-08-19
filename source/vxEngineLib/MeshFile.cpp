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

#include <vxEngineLib/MeshFile.h>
#include <vxLib/util/CityHash.h>
#include <vxLib/memory.h>
#include <vxLib/File/File.h>
#include <vxEngineLib/ArrayAllocator.h>
#include "ConverterMeshFileV0.h"
#include "ConverterMeshFileV1.h"

namespace vx
{
	MeshFile::MeshFile(u32 version)
		:Serializable(version),
		m_mesh(),
		m_meshData(),
		m_physxData(),
		m_physxDataSize(0),
		m_physxMeshType()
	{

	}

	MeshFile::MeshFile(MeshFile &&rhs)
		:Serializable(std::move(rhs)),
		m_mesh(std::move(rhs.m_mesh)),
		m_meshData(std::move(m_meshData)),
		m_physxData(std::move(rhs.m_physxData)),
		m_physxDataSize(rhs.m_physxDataSize),
		m_physxMeshType(rhs.m_physxMeshType)
	{
		rhs.m_physxDataSize = 0;
	}

	MeshFile::MeshFile(u32 version, vx::Mesh &&mesh, managed_ptr<u8[]> &&meshData, managed_ptr<u8[]> &&physxData, u32 physxDataSize, PhsyxMeshType meshType)
		:Serializable(version), 
		m_mesh(std::move(mesh)),
		m_meshData(std::move(meshData)),
		m_physxData(std::move(physxData)),
		m_physxDataSize(physxDataSize),
		m_physxMeshType(meshType)
	{
	}

	MeshFile::~MeshFile()
	{
		m_physxDataSize = 0;
	}

	MeshFile& MeshFile::operator = (MeshFile &&rhs)
	{
		if (this != &rhs)
		{
			Serializable::operator=(std::move(rhs));
			m_mesh.swap(rhs.m_mesh);
			m_meshData.swap(rhs.m_meshData);
			m_physxData.swap(rhs.m_physxData);
			std::swap(m_physxDataSize, rhs.m_physxDataSize);
			std::swap(m_physxMeshType, rhs.m_physxMeshType);
		}
		return *this;
	}

	const u8* MeshFile::loadFromMemory(const u8 *ptr, u32 size, vx::Allocator* allocator)
	{
		return loadFromMemory(ptr, size, (ArrayAllocator*)allocator);
	}

	const u8* MeshFile::loadFromMemory(const u8 *ptr, u32 size, ArrayAllocator* allocator)
	{
		auto version = getVersion();
		if (version == 0)
		{
			ptr = Converter::MeshFileV0::loadFromMemory(ptr, size, allocator, this);
		}
		else if (version == 1)
		{
			ptr = Converter::MeshFileV1::loadFromMemory(ptr, size, allocator, this);
		}

		setVersion(getGlobalVersion());

		return ptr;
	}

	void MeshFile::saveToFile(vx::File* file) const
	{
		m_mesh.saveToFile(file);
		file->write(m_physxDataSize);
		file->write(m_physxMeshType);
		file->write(m_physxData.get(), m_physxDataSize);
	}

	u64 MeshFile::getCrc() const
	{
		auto version = getVersion();
		
		u64 result = 0;
		if (version == 0)
		{
			result = Converter::MeshFileV0::getCrc(*this);
		}
		else if (version == 1)
		{
			result = Converter::MeshFileV1::getCrc(*this);
		}

		return version;
	}

	u32 MeshFile::getGlobalVersion()
	{
		return 1;
	}

	void MeshFile::setPhysxMesh(managed_ptr<u8[]> &&physxData, u32 physxDataSize, PhsyxMeshType meshType)
	{
		m_physxData = std::move(physxData);
		m_physxDataSize = physxDataSize;
		m_physxMeshType = meshType;
	}

	const u8* MeshFile::getPhysxData() const
	{
		return m_physxData.get();
	}

	PhsyxMeshType MeshFile::getPhysxMeshType() const
	{
		return m_physxMeshType;
	}
}