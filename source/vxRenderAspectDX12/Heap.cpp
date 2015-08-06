#include "Heap.h"
#include <d3d12.h>
#include "Device.h"

namespace d3d
{
	Heap::Heap()
		:m_heap()
	{

	}

	Heap::~Heap()
	{
	}

	bool Heap::create(const D3D12_HEAP_DESC &desc, Device* device)
	{
		auto d3dDevice = device->getDevice();

		return (d3dDevice->CreateHeap(&desc, IID_PPV_ARGS(m_heap.getAddressOf())) == 0);
	}

	void Heap::destroy()
	{
		m_heap.destroy();
	}

	bool Heap::createBufferHeap(u64 size, D3D12_HEAP_TYPE type, Device* device)
	{
		auto d3dDevice = device->getDevice();
		size = (size + 0xffff) & ~0xffff;

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
		desc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;

		return (d3dDevice->CreateHeap(&desc, IID_PPV_ARGS(m_heap.getAddressOf())) == 0);
	}

	bool Heap::createResource(const D3D12_RESOURCE_DESC &desc, u64 offset, D3D12_RESOURCE_STATES state, ID3D12Resource** resource, Device* device)
	{
		auto d3dDevice = device->getDevice();

		return (d3dDevice->CreatePlacedResource(m_heap.get(), offset, &desc, state, nullptr, IID_PPV_ARGS(resource)) == 0);
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