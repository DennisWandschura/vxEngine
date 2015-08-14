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

struct ID3D12DescriptorHeap;
struct D3D12_DESCRIPTOR_HEAP_DESC;
struct D3D12_CPU_DESCRIPTOR_HANDLE;

enum D3D12_DESCRIPTOR_HEAP_TYPE;

#include "d3d.h"

namespace d3d
{
	class DescriptorHeap;
	class Device;

	struct DescriptorHandleCpu
	{
		u64 m_ptr;
		u32 m_size;

		explicit DescriptorHandleCpu(DescriptorHeap* heap);
		DescriptorHandleCpu(u64 ptr, u32 size);

		void offset(u32 count)
		{
			m_ptr += count * m_size;
		}

		operator D3D12_CPU_DESCRIPTOR_HANDLE();
	};

	class DescriptorHeap
	{
		Object<ID3D12DescriptorHeap> m_heap;
		u32 m_incrementSize;
		u32 m_type;

	public:
		DescriptorHeap();
		~DescriptorHeap();

		bool create(const D3D12_DESCRIPTOR_HEAP_DESC &desc, Device* device);
		void destroy();

		ID3D12DescriptorHeap* operator->() { return m_heap.get(); }
		ID3D12DescriptorHeap* get() { return m_heap.get(); }

		u32 getIncrementSize() const { return m_incrementSize; }

		DescriptorHandleCpu getHandleCpu();
	};
}