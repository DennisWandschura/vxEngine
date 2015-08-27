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
struct ID3D12Device;

enum D3D12_HEAP_TYPE;

#include "d3d.h"
#include <memory>
#include "CreateResourceDesc.h"

namespace d3d
{
	struct HeapCreateBufferResourceDesc
	{
		u64 size;
		ID3D12Resource** resource;
		D3D12_RESOURCE_STATES state;
	};

	struct HeapCreateResourceDesc
	{
		CreateResourceDesc desc;
		ID3D12Resource** resource;
	};

	class Heap
	{
		struct Page;

		Object<ID3D12Heap> m_heap;
		ID3D12Device* m_device;
		u64 m_offset;
		u64 m_capacity;

		bool createHeap(u32 flags, u64 size, D3D12_HEAP_TYPE type, ID3D12Device* device);
		bool createBuffer(const HeapCreateBufferResourceDesc &desc);

	public:
		Heap(); 
		~Heap();

		bool create(const D3D12_HEAP_DESC &desc, ID3D12Device* device);
		void destroy();

		void setName(const wchar_t* name);

		bool createBufferHeap(u64 size, D3D12_HEAP_TYPE type, ID3D12Device* device);
		bool createTextureHeap(u64 size, D3D12_HEAP_TYPE type, ID3D12Device* device);
		bool createRtHeap(u64 size, D3D12_HEAP_TYPE type, ID3D12Device* device);

		bool createResource(const HeapCreateResourceDesc &desc);
		bool createResources(const HeapCreateResourceDesc* desc, u32 count);
		bool createResourceBuffer(const HeapCreateBufferResourceDesc &desc);
		bool createResourceBuffer(const HeapCreateBufferResourceDesc* desc, u32 count);

		ID3D12Heap* get() { return m_heap.get(); }
	};
}