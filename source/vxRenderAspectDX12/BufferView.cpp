#include "BufferView.h"
#include <vxLib/types.h>
#include <d3d12.h>
#include "Device.h"
#include "DescriptorHeap.h"

namespace d3d
{

	void BufferView::createConstantBufferView(const D3D12_CONSTANT_BUFFER_VIEW_DESC &desc, DescriptorHandleCpu handle, Device* device)
	{
		auto d3dDevice = device->getDevice();

		D3D12_CPU_DESCRIPTOR_HANDLE d3dHandle;
		d3dHandle.ptr = handle.m_ptr;

		d3dDevice->CreateConstantBufferView(&desc, d3dHandle);
	}

	void BufferView::createShaderResourceView(ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC &desc, DescriptorHandleCpu handle, Device* device)
	{
		auto d3dDevice = device->getDevice();
		D3D12_CPU_DESCRIPTOR_HANDLE d3dHandle;
		d3dHandle.ptr = handle.m_ptr;

		d3dDevice->CreateShaderResourceView(resource, &desc, d3dHandle);
	}
}