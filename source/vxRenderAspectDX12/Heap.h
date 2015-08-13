#pragma once

struct ID3D12Heap;
struct ID3D12Resource;
struct D3D12_HEAP_DESC;
struct D3D12_RESOURCE_DESC;

enum D3D12_HEAP_TYPE;
enum D3D12_RESOURCE_STATES;

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

	public:
		Heap(); 
		~Heap();

		bool create(const D3D12_HEAP_DESC &desc, Device* device);
		void destroy();

		bool createBufferHeap(u64 size, D3D12_HEAP_TYPE type, Device* device);

		bool createResource(const D3D12_RESOURCE_DESC &desc, u64 offset, D3D12_RESOURCE_STATES state, ID3D12Resource** resource, Device* device);
		bool createResourceBuffer(u64 size, u64 offset, D3D12_RESOURCE_STATES state, ID3D12Resource** resource, Device* device);

		ID3D12Heap* get() { return m_heap.get(); }
	};
}