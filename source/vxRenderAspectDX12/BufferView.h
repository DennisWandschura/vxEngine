#pragma once

struct D3D12_CONSTANT_BUFFER_VIEW_DESC;
struct D3D12_SHADER_RESOURCE_VIEW_DESC;
struct ID3D12Resource;

namespace d3d
{
	struct DescriptorHandleCpu;
	class Device;

	struct BufferView
	{
		static void createConstantBufferView(const D3D12_CONSTANT_BUFFER_VIEW_DESC &desc, DescriptorHandleCpu handle, Device* device);
		static void createShaderResourceView(ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC &desc, DescriptorHandleCpu handle, Device* device);
	};
}