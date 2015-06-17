#pragma once
#ifndef __VX_SERIALIZABLE_H
#define __VX_SERIALIZABLE_H
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

namespace vx
{
	class Allocator;
}

#include <vxLib/types.h>

namespace vx
{
	class File;

	class Serializable
	{
		u32 m_version;

	public:
		explicit Serializable(u32 version) :m_version(version){}
		Serializable(const Serializable &rhs) :m_version(rhs.m_version){}
		Serializable(Serializable &&rhs) :m_version(rhs.m_version){}

		virtual ~Serializable(){}

		Serializable& operator=(const Serializable &rhs)
		{
			m_version = rhs.m_version;
			return *this;
		}

		Serializable& operator=(Serializable &&rhs)
		{
			m_version = rhs.m_version;
			return *this;
		}

		virtual void saveToFile(File* f) const = 0;

		virtual const u8* loadFromMemory(const u8 *ptr, u32 size, vx::Allocator* allocator) = 0;

		virtual u64 getCrc() const = 0;

		u32 getVersion() const { return m_version; }
	};
}
#endif