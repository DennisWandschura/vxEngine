#pragma once
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

#include <vxEngineLib/Serializable.h>

class ActorFile : public vx::Serializable
{
	char m_mesh[32];
	char m_material[32];

public:
	explicit ActorFile(u32 version);
	~ActorFile();

	void saveToFile(vx::File* f) const override;

	const u8* loadFromMemory(const u8 *ptr, u32 size, vx::Allocator* allocator) override;

	u64 getCrc() const override;

	void setMesh(const char(&mesh)[32]);
	void setMaterial(const char(&material)[32]);

	const char* getMesh() { return m_mesh; }
	const char* getMaterial() { return m_material; }

	static u32 getGlobalVersion() { return 0; }
};