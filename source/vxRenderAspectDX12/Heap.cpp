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
#include "Device.h"

namespace d3d
{
	struct Heap::Page
	{
		u32 offset;
		u32 size;
	};

	Heap::Heap()
		:m_heap(),
		m_pages(),
		m_pageCount(0),
		m_pageSize(0)
	{

	}

	Heap::~Heap()
	{
	}

	void Heap::init(u64 size, u64 alignment)
	{
		m_pageSize = alignment;
		m_pageCount = size / m_pageSize;
		m_pages = std::make_unique<Page[]>(m_pageCount);

		u32 offset = 0;
		for (u32 i = 0; i < m_pageCount; ++i)
		{
			m_pages[i].offset = offset;
			m_pages[i].size = m_pageSize;

			offset += m_pageSize;
		}
	}

	bool Heap::create(const D3D12_HEAP_DESC &desc, Device* device)
	{
		auto d3dDevice = device->getDevice();

		auto result = (d3dDevice->CreateHeap(&desc, IID_PPV_ARGS(m_heap.getAddressOf())) == 0);
		if (result)
		{
			init(desc.SizeInBytes, desc.Alignment);
		}

		return result;
	}

	void Heap::destroy()
	{
		m_heap.destroy();
	}

	bool Heap::createHeap(u32 flags, u64 size, D3D12_HEAP_TYPE type, Device* device)
	{
		auto d3dDevice = device->getDevice();

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

	bool Heap::createBufferHeap(u64 size, D3D12_HEAP_TYPE type, Device* device)
	{
		auto d3dDevice = device->getDevice();
		size = (size + 0xffff) & ~0xffff;

		return createHeap(D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS, size, type, device);
	}

	bool Heap::createTextureHeap(u64 size, D3D12_HEAP_TYPE type, Device* device)
	{
		auto d3dDevice = device->getDevice();
		size = (size + 0xffff) & ~0xffff;

		return createHeap(D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES, size, type, device);
	}

	bool Heap::createRtHeap(u64 size, D3D12_HEAP_TYPE type, Device* device)
	{
		auto d3dDevice = device->getDevice();
		size = (size + 0xffff) & ~0xffff;

		return createHeap(D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES, size, type, device);
	}

	bool Heap::createResource(const D3D12_RESOURCE_DESC &desc, u64 offset, D3D12_RESOURCE_STATES state, const D3D12_CLEAR_VALUE* clearValue, ID3D12Resource** resource, Device* device)
	{
		auto d3dDevice = device->getDevice();

		auto hresult = d3dDevice->CreatePlacedResource(m_heap.get(), offset, &desc, state, clearValue, IID_PPV_ARGS(resource));
		return (hresult == 0);
	}

	bool Heap::createResourceBuffer(u64 size, u64 offset, D3D12_RESOURCE_STATES state, ID3D12Resource** resource, Device* device)
	{
		size = (size + 0xffff) & ~0xffff;
		offset = (offset + 0xffff) & ~0xffff;

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
		rdesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		auto d3dDevice = device->getDevice();
		return (d3dDevice->CreatePlacedResource(m_heap.get(), offset, &rdesc, state, nullptr, IID_PPV_ARGS(resource)) == 0);
	}
}