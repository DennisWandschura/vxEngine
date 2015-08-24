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

struct ID3D12Heap;
struct ID3D12Resource;
struct D3D12_HEAP_DESC;
struct D3D12_RESOURCE_DESC;

enum D3D12_HEAP_TYPE;
enum D3D12_RESOURCE_STATES;
struct D3D12_CLEAR_VALUE;

#include "d3d.h"
#include <memory>

namespace d3d
{
	class Device;

	class Heap
	{
		struct Page;

		Object<ID3D12Heap> m_heap;
		std::unique_ptr<Page[]> m_pages;
		u32 m_pageCount;
		u32 m_pageSize;

		void init(u64 size, u64 alignment);

		bool createHeap(u32 flags, u64 size, D3D12_HEAP_TYPE type, Device* device);

	public:
		Heap(); 
		~Heap();

		bool create(const D3D12_HEAP_DESC &desc, Device* device);
		void destroy();

		bool createBufferHeap(u64 size, D3D12_HEAP_TYPE type, Device* device);
		bool createTextureHeap(u64 size, D3D12_HEAP_TYPE type, Device* device);
		bool createRtHeap(u64 size, D3D12_HEAP_TYPE type, Device* device);

		bool createResource(const D3D12_RESOURCE_DESC &desc, u64 offset, D3D12_RESOURCE_STATES state, const D3D12_CLEAR_VALUE* clearValue, ID3D12Resource** resource, Device* device);
		bool createResourceBuffer(u64 size, u64 offset, D3D12_RESOURCE_STATES state, ID3D12Resource** resource, Device* device);

		ID3D12Heap* get() { return m_heap.get(); }
	};
}