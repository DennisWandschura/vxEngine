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

#include <vxLib/Graphics/Mesh.h>
#include "Serializable.h"

namespace vx
{
	class MeshFile : public Serializable
	{
		vx::Mesh m_mesh;
		const u8* m_physxData;
		u32 m_physxDataSize;

	public:
		MeshFile();
		MeshFile(const MeshFile&) = delete;
		MeshFile(MeshFile &&rhs);
		MeshFile(vx::Mesh &&mesh, const u8* physxData, u32 physxDataSize);
		~MeshFile();

		MeshFile& operator=(const MeshFile&) = delete;
		MeshFile& operator=(MeshFile &&rhs);

		const u8* loadFromMemory(const u8 *ptr, u32 version, vx::Allocator* allocator) override;

		bool saveToFile(vx::File* file) const;

		u64 getCrc() const override;

		u32 getVersion() const override;

		const vx::Mesh& getMesh() const { return m_mesh; }
		u32 getPhysxDataSize() const { return m_physxDataSize; }
		const u8* getPhysxData() const { return m_physxData; }
	};
}
#endif