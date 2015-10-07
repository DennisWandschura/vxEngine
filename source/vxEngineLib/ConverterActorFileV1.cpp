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

#include "ConverterActorFileV1.h"
#include <vxEngineLib/ActorFile.h>
#include <vxLib/util/CityHash.h>
#include <vxLib/string.h>

namespace Converter
{
	const u8* ActorFileV1::loadFromMemory(const u8 *ptr, u32 size, ArrayAllocator* allocator, ActorFile* actorFile)
	{
		ptr = vx::read(actorFile->m_mesh, ptr);
		ptr = vx::read(actorFile->m_material, ptr);
		ptr = vx::read(actorFile->m_fovRad, ptr);

		return ptr;
	}

	u64 ActorFileV1::getCrc(const ActorFile &actorFile)
	{
		const auto bufferSize = sizeof(actorFile.m_mesh) + sizeof(actorFile.m_material) + sizeof(actorFile.m_fovRad);
		u8 buffer[bufferSize];
		memset(buffer, 0, bufferSize);

		auto p = vx::write(buffer, actorFile.m_mesh);
		p = vx::write(p, actorFile.m_material);
		p = vx::write(p, actorFile.m_fovRad);

		return CityHash64((char*)buffer, bufferSize);
	}
}