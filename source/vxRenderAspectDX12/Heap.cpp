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
#include "Heap.h"
#include <d3d12.h>

namespace d3d
{
	struct Heap::Page
	{
		u32 offset;
		u32 size;
	};

	Heap::Heap()
		:m_heap(),
		m_device(nullptr),
		m_offset(0),
		m_capacity(0)
	{

	}

	Heap::~Heap()
	{
	}

	bool Heap::create(const D3D12_HEAP_DESC &desc, ID3D12Device* device)
	{
		auto result = (device->CreateHeap(&desc, IID_PPV_ARGS(m_heap.getAddressOf())) == 0);
		if (result)
		{
			m_device = device;
			m_offset = 0;
			m_capacity = desc.SizeInBytes;
		}

		return result;
	}

	void Heap::destroy()
	{
		m_heap.destroy();
		m_device = nullptr;
	}

	void Heap::setName(const wchar_t* name)
	{
		m_heap->SetName(name);
	}

	bool Heap::createHeap(u32 flags, u64 size, D3D12_HEAP_TYPE type, ID3D12Device* device)
	{
		D3D12_HEAP_PROPERTIES props
		{
			type,
			D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
			D3D12_MEMORY_POOL_UNKNOWN,
			0,
			0
		};

		D3D12_HEAP_DESC desc;
		desc.Alignment = 64 KBYTE;
		desc.Properties = props;
		desc.SizeInBytes = size;
		desc.Flags = (D3D12_HEAP_FLAGS)flags;

		return create(desc, device);
	}

	bool Heap::createBufferHeap(u64 size, D3D12_HEAP_TYPE type, ID3D12Device* device)
	{
		size = (size + 0xffff) & ~0xffff;

		return createHeap(D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS, size, type, device);
	}

	bool Heap::createTextureHeap(u64 size, D3D12_HEAP_TYPE type, ID3D12Device* device)
	{
		size = (size + 0xffff) & ~0xffff;

		return createHeap(D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES, size, type, device);
	}

	bool Heap::createRtHeap(u64 size, D3D12_HEAP_TYPE type, ID3D12Device* device)
	{
		size = (size + 0xffff) & ~0xffff;

		return createHeap(D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES, size, type, device);
	}

	bool Heap::createResource(const HeapCreateResourceDesc &desc)
	{
		auto d3dDevice = m_device;
		auto alignment = desc.desc.resDesc->Alignment;
		auto offset = d3d::getAlignedSize(m_offset, alignment);

		auto hresult = d3dDevice->CreatePlacedResource(m_heap.get(), offset, desc.desc.resDesc, desc.desc.state, desc.desc.clearValue, IID_PPV_ARGS(desc.resource));
		auto result = (hresult == 0);
		if (result)
		{
			m_offset = offset + desc.desc.size;
		}
		else
		{
			puts("");
		}

		return result;
	}

	bool Heap::createResources(const HeapCreateResourceDesc* desc, u32 count)
	{
		for (u32 i = 0; i < count; ++i)
		{
			if (!createResource(desc[i]))
				return false;
		}

		return true;
	}

	bool Heap::createBuffer(const HeapCreateBufferResourceDesc &desc)
	{
		auto size = (desc.size + 0xffff) & ~0xffff;
		auto offset = m_offset;

		D3D12_RESOURCE_DESC rdesc = {};
		rdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rdesc.Alignment = 64 KBYTE;
		rdesc.Width = size;
		rdesc.Height = 1;
		rdesc.DepthOrArraySize = 1;
		rdesc.MipLevels = 1;
		rdesc.Format = DXGI_FORMAT_UNKNOWN;
		rdesc.SampleDesc.Count = 1;
		rdesc.SampleDesc.Quality = 0;
		rdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		rdesc.Flags = desc.flags;

		HeapCreateResourceDesc heapResDesc;
		heapResDesc.desc.clearValue = nullptr;
		heapResDesc.desc.resDesc = &rdesc;
		heapResDesc.resource = desc.resource;
		heapResDesc.desc.size = size;
		heapResDesc.desc.state = desc.state;

		return createResource(heapResDesc);
	}

	bool Heap::createResourceBuffer(const HeapCreateBufferResourceDesc &desc)
	{
		return createBuffer(desc);
	}

	bool Heap::createResourceBuffer(const HeapCreateBufferResourceDesc* desc, u32 count)
	{
		for (u32 i = 0; i < count; ++i)
		{
			if (!createBuffer(desc[i]))
				return false;
		}

		return true;
	}
}