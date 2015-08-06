#pragma once

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