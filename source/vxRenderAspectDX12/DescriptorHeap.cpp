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
#include "DescriptorHeap.h"
#include <d3d12.h>
#include "Device.h"

namespace d3d
{
	DescriptorHandleCpu::DescriptorHandleCpu(DescriptorHeap* heap)
		:m_ptr(0),
		m_size(0)
	{
		auto handle = (*heap)->GetCPUDescriptorHandleForHeapStart();

		m_ptr = handle.ptr;
		m_size = heap->getIncrementSize();
	}

	DescriptorHandleCpu::DescriptorHandleCpu(u64 ptr, u32 size)
		:m_ptr(ptr),
		m_size(size)
	{

	}

	DescriptorHandleCpu::operator D3D12_CPU_DESCRIPTOR_HANDLE()
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle;
		handle.ptr = m_ptr;

		return handle;
	}

	DescriptorHeap::DescriptorHeap()
		:m_heap(),
		m_incrementSize(0),
		m_type(0)
	{

	}

	DescriptorHeap::~DescriptorHeap()
	{

	}

	bool DescriptorHeap::create(const D3D12_DESCRIPTOR_HEAP_DESC &desc, ID3D12Device* device)
	{
		bool result = true;

		if (m_heap.get() == nullptr)
		{
			result = (device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_heap.getAddressOf())) == 0);
			if (result)
			{
				m_incrementSize = device->GetDescriptorHandleIncrementSize(desc.Type);
				m_type = desc.Type;
			}
		}

		return result;
	}

	void DescriptorHeap::destroy()
	{
		m_heap.destroy();
	}

	DescriptorHandleCpu DescriptorHeap::getHandleCpu()
	{
		return DescriptorHandleCpu(m_heap->GetCPUDescriptorHandleForHeapStart().ptr, m_incrementSize);
	}
}