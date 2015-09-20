#pragma once

#include <vxLib/types.h>
#include <d3d12.h>

namespace d3d
{
	struct ShaderResourceViewDesc : public D3D12_SHADER_RESOURCE_VIEW_DESC
	{
		operator D3D12_SHADER_RESOURCE_VIEW_DESC&()
		{
			return *this;
		}

		operator const D3D12_SHADER_RESOURCE_VIEW_DESC&() const
		{
			return *this;
		}

		static ShaderResourceViewDesc getDescTexture2D(DXGI_FORMAT format, u32 mipLevels = 1)
		{
			ShaderResourceViewDesc desc;
			desc.Format = format;
			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipLevels = mipLevels;
			desc.Texture2D.MostDetailedMip = 0;
			desc.Texture2D.PlaneSlice = 0;
			desc.Texture2D.ResourceMinLODClamp = 0;

			return desc;
		}

		static ShaderResourceViewDesc getDescTexture3D(DXGI_FORMAT format, u32 mipLevels = 1)
		{
			ShaderResourceViewDesc srvDesc;
			srvDesc.Format = format;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			srvDesc.Texture3D.MipLevels = mipLevels;
			srvDesc.Texture3D.ResourceMinLODClamp = 0;

			return srvDesc;
		}
	};
}