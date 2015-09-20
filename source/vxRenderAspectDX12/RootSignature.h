#pragma once

#include <vxLib/types.h>
#include <d3d12.h>
#include "d3dObject.h"

namespace d3d
{
	class RootSignature
	{
		Object<ID3D12RootSignature> m_rootSignature;

	public:
		RootSignature() :m_rootSignature() {}

		bool create(const D3D12_ROOT_SIGNATURE_DESC* desc,ID3D12Device* device)
		{
			ID3DBlob* blob = nullptr;
			ID3DBlob* errorBlob = nullptr;
			auto hresult = D3D12SerializeRootSignature(desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &errorBlob);
			if (hresult != 0)
				return false;

			hresult = device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.getAddressOf()));
			if (hresult != 0)
				return false;

			return true;
		}

		void destroy()
		{
			m_rootSignature.destroy();
		}

		ID3D12RootSignature* get() { return m_rootSignature.get(); }
		const ID3D12RootSignature* get() const { return m_rootSignature.get(); }
	};
}