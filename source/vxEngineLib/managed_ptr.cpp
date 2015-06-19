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
#include <vxEngineLib/managed_ptr.h>
#include <vxEngineLib/ArrayAllocator.h>

managed_ptr_base::managed_ptr_base(managed_ptr_base &&rhs)
	:m_ptr(rhs.m_ptr),
	entryIndex(rhs.entryIndex),
	alloc(rhs.alloc)
{
	updateAllocator();

	rhs.m_ptr = nullptr;
	rhs.alloc = nullptr;
}

managed_ptr_base& managed_ptr_base::operator = (managed_ptr_base &&rhs)
{
	if (this != &rhs)
	{
		auto tmp = m_ptr;
		auto tmpIndex = entryIndex;
		auto tmpAlloc = alloc;

		m_ptr = rhs.m_ptr;
		entryIndex = rhs.entryIndex;
		alloc = rhs.alloc;

		rhs.m_ptr = tmp;
		rhs.entryIndex = tmpIndex;
		rhs.alloc = tmpAlloc;

		updateAllocator();
		rhs.updateAllocator();
	}

	return *this;
}

void managed_ptr_base::updateAllocator()
{
	if (alloc != nullptr)
	{
		alloc->updateEntry(entryIndex, this);
	}
}

unsigned managed_ptr_base::getEntrySize() const
{
	return alloc->getEntrySize(entryIndex);
}