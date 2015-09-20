#pragma once

#include <vxLib/math/Vector.h>
#include <d3d12.h>

namespace d3d
{
	struct ResourceDesc : public D3D12_RESOURCE_DESC
	{
		operator D3D12_RESOURCE_DESC&()
		{
			return *this;
		}

		operator const D3D12_RESOURCE_DESC&() const
		{
			return *this;
		}

		static ResourceDesc getDescTexture2D(const vx::uint2& resolution, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, u32 mipLevels = 1)
		{
			ResourceDesc desc;
			desc.Alignment = 64 KBYTE;
			desc.DepthOrArraySize = 1;
			desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			desc.Flags = flags;
			desc.Format = format;
			desc.Height = resolution.y;
			desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			desc.MipLevels = mipLevels;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Width = resolution.x;

			return desc;
		}
	};
}