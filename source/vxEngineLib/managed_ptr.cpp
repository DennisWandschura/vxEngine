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

managed_ptr_base::managed_ptr_base()
	:m_ptr(),
	m_alloc(),
	m_entryIndex()
{

}

managed_ptr_base::managed_ptr_base(u8* p, ArrayAllocator* alloc, unsigned index)
	:m_ptr(p), m_alloc(alloc), m_entryIndex(index)
{
}

managed_ptr_base::managed_ptr_base(managed_ptr_base &&rhs)
	:m_ptr(rhs.m_ptr),
	m_alloc(rhs.m_alloc),
	m_entryIndex(rhs.m_entryIndex)
{
	updateAllocator();

	rhs.m_ptr = nullptr;
	rhs.m_alloc = nullptr;
}

managed_ptr_base& managed_ptr_base::operator = (managed_ptr_base &&rhs)
{
	if (this != &rhs)
	{
		swap(rhs);
	}

	return *this;
}

void managed_ptr_base::updateAllocator()
{
	if (m_alloc != nullptr)
	{
		m_alloc->updateEntry(m_entryIndex, this);
	}
}

unsigned managed_ptr_base::getEntrySize() const
{
	return m_alloc->getEntrySize(m_entryIndex);
}

void managed_ptr_base::swap(managed_ptr_base &other)
{
	std::swap(m_ptr, other.m_ptr);
	std::swap(m_alloc, other.m_alloc);
	std::swap(m_entryIndex, other.m_entryIndex);

	updateAllocator();
	other.updateAllocator();
}