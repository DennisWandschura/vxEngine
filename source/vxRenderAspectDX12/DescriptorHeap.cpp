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

	bool DescriptorHeap::create(const D3D12_DESCRIPTOR_HEAP_DESC &desc, Device* device)
	{
		bool result = true;

		if (m_heap.get() == nullptr)
		{
			result = (device->getDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_heap.getAddressOf())) == 0);
			if (result)
			{
				m_incrementSize = device->getDevice()->GetDescriptorHandleIncrementSize(desc.Type);
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