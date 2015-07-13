#pragma once
#ifndef __VX_MESHFILE_H
#define __VX_MESHFILE_H
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

class ArrayAllocator;

#include <vxLib/Graphics/Mesh.h>
#include "Serializable.h"
#include <vxEngineLib/managed_ptr.h>

namespace vx
{
	enum class PhsyxMeshType : u32 { Triangle, Convex };

	class MeshFile : public Serializable
	{
		vx::Mesh m_mesh;
		managed_ptr<u8[]> m_meshData;
		managed_ptr<u8[]> m_physxData;
		u32 m_physxDataSize;
		PhsyxMeshType m_physxMeshType;

		const u8* loadFromMemoryV0(const u8 *ptr, u32 size, ArrayAllocator* allocator);
		const u8* loadFromMemoryV1(const u8 *ptr, u32 size, ArrayAllocator* allocator);

		const u8* loadFromMemory(const u8 *ptr, u32 size, vx::Allocator* allocator) override;

	public:
		explicit MeshFile(u32 version);
		MeshFile(const MeshFile&) = delete;
		MeshFile(MeshFile &&rhs);
		MeshFile(u32 version, vx::Mesh &&mesh, managed_ptr<u8[]> &&meshData, managed_ptr<u8[]> &&physxData, u32 physxDataSize, PhsyxMeshType meshType);
		~MeshFile();

		MeshFile& operator=(const MeshFile&) = delete;
		MeshFile& operator=(MeshFile &&rhs);

		const u8* loadFromMemory(const u8 *ptr, u32 size, ArrayAllocator* allocator);

		void saveToFile(vx::File* file) const override;

		u64 getCrc() const override;

		static u32 getGlobalVersion();

		const vx::Mesh& getMesh() const { return m_mesh; }
		u32 getPhysxDataSize() const { return m_physxDataSize; }
		const u8* getPhysxData() const;

		PhsyxMeshType getPhysxMeshType() const;
	};
}
#endif