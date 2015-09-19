#include "RenderPassVoxelMip.h"
#include "ShaderManager.h"
#include "ResourceManager.h"
#include "d3dx12.h"
#include "CommandAllocator.h"

RenderPassVoxelMip::RenderPassVoxelMip(d3d::CommandAllocator* cmdAlloc)
	:m_cmdAlloc(cmdAlloc)
{

}

RenderPassVoxelMip::~RenderPassVoxelMip()
{

}

void RenderPassVoxelMip::getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device)
{

}

bool RenderPassVoxelMip::createData(ID3D12Device* device)
{
	return true;
}

bool RenderPassVoxelMip::loadShaders()
{
	if (!s_shaderManager->loadShader("VoxelMipVS.cso", L"../../lib/VoxelMipVS.cso", d3d::ShaderType::Vertex))
		return false;

	//if (!s_shaderManager->loadShader("VoxelMipOpacityVS.cso", L"../../lib/VoxelMipOpacityVS.cso", d3d::ShaderType::Vertex))
	//	return false;

	return true;
}

bool RenderPassVoxelMip::createRootSignature(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangeVS[2];
	rangeVS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, 0);
	rangeVS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, 1);

	CD3DX12_ROOT_PARAMETER rootParameters[3];
	rootParameters[0].InitAsDescriptorTable(1, &rangeVS[0], D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsDescriptorTable(1, &rangeVS[1], D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[2].InitAsConstants(2, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(3, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob* blob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	auto hresult = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &errorBlob);
	if (hresult != 0)
		return false;

	hresult = device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.getAddressOf()));
	if (hresult != 0)
		return false;

	return true;
}

bool RenderPassVoxelMip::createPipelineState(ID3D12Device* device)
{
	d3d::PipelineStateDescInput inputDesc;
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	inputDesc.rootSignature = m_rootSignature.get();
	inputDesc.shaderDesc.vs = s_shaderManager->getShader("VoxelMipVS.cso");
	inputDesc.rtvCount = 0;
	inputDesc.depthEnabled = 0;

	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);

	return m_pipelineState.create(desc, &m_pipelineState, device);
}

void RenderPassVoxelMip::createViews(ID3D12Device* device)
{
	auto voxelTextureColor = s_resourceManager->getTexture(L"voxelTextureColor");
	auto texDesc = voxelTextureColor->GetDesc();

	auto handle = m_uavHeap.getHandleCpu();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
	srvDesc.Texture3D.MipLevels = 1;
	srvDesc.Texture3D.ResourceMinLODClamp = 0;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;

	u32 mipLevels = texDesc.MipLevels;

	u32 wSize = texDesc.DepthOrArraySize;

	for (u32 i = 0; i < mipLevels - 1; ++i)
	{
		u32 dstMip = i + 1;

		auto dstWSize = wSize >> dstMip;
		uavDesc.Texture3D.FirstWSlice = 0;
		uavDesc.Texture3D.WSize = dstWSize;
		uavDesc.Texture3D.MipSlice = dstMip;

		srvDesc.Texture3D.MostDetailedMip = i;
		device->CreateShaderResourceView(voxelTextureColor->get(), &srvDesc, handle);

		handle.offset(1);
		device->CreateUnorderedAccessView(voxelTextureColor->get(), nullptr, &uavDesc, handle);

		handle.offset(1);
	}
}

bool RenderPassVoxelMip::initialize(ID3D12Device* device, void* p)
{
	if (!loadShaders())
		return false;

	if (!createRootSignature(device))
		return false;

	if (!createPipelineState(device))
		return false;

	auto mipLevels = s_settings->m_lpvMip;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	heapDesc.NodeMask = 1;
	heapDesc.NumDescriptors = 2 * mipLevels;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (!m_uavHeap.create(heapDesc, device))
	{
		return false;
	}

	if (!m_commandList.create(device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAlloc->get(), m_pipelineState.get()))
		return false;

	createViews(device);

	return true;
}

void RenderPassVoxelMip::shutdown()
{

}

void RenderPassVoxelMip::submitCommands(Graphics::CommandQueue* queue)
{
	auto voxelTextureColor = s_resourceManager->getTexture(L"voxelTextureColor");

	const u32 voxelDim = s_settings->m_lpvDim;
	auto mipLevels = s_settings->m_lpvMip;

	m_commandList->Reset(m_cmdAlloc->get(), m_pipelineState.get());

	auto heap = m_uavHeap.get();
	m_commandList->SetDescriptorHeaps(1, &heap);
	m_commandList->SetGraphicsRootSignature(m_rootSignature.get());

	auto gpuHandle = m_uavHeap.getHandleGpu();
	

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(voxelTextureColor->get()));

	for (u32 axis = 0; axis < 6; ++axis)
	{
		auto gpuHandleUav = gpuHandle;
		for (u32 i = 0; i < mipLevels - 1; ++i)
		{
			u32 srcMip = i;
			u32 dstMip = i + 1;
			u32 dstDim = voxelDim >> dstMip;

			m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(voxelTextureColor->get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, srcMip));

			m_commandList->SetGraphicsRootDescriptorTable(0, gpuHandleUav);
			m_commandList->SetGraphicsRootDescriptorTable(1, gpuHandleUav);

			vx::uint2 rootData(axis, srcMip);
			m_commandList->SetGraphicsRoot32BitConstants(2, 2, &rootData, 0);

			m_commandList->DrawInstanced(dstDim, dstDim * dstDim, 0, 0);

			m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(voxelTextureColor->get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, srcMip));
			m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(voxelTextureColor->get()));

			gpuHandleUav.offset(2);
		}
	}

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(voxelTextureColor->get()));

	m_commandList->Close();

	queue->pushCommandList(&m_commandList);
}