#include "RenderPassVoxelMip.h"
#include "ShaderManager.h"
#include "ResourceManager.h"
#include "d3dx12.h"

RenderPassVoxelMip::RenderPassVoxelMip(ID3D12CommandAllocator* cmdAlloc)
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
	if (!s_shaderManager->loadShader("VoxelizeCreateMipVS.cso", L"../../lib/VoxelizeCreateMipVS.cso", d3d::ShaderType::Vertex))
		return false;

	return true;
}

bool RenderPassVoxelMip::createRootSignature(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangeVS[2];
	rangeVS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, 0);
	rangeVS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, 1);

	CD3DX12_ROOT_PARAMETER rootParameters[1];
	rootParameters[0].InitAsDescriptorTable(2, rangeVS, D3D12_SHADER_VISIBILITY_VERTEX);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(1, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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
	inputDesc.shaderDesc.vs = s_shaderManager->getShader("VoxelizeCreateMipVS.cso");
	inputDesc.rtvCount = 0;
	inputDesc.depthEnabled = 0;

	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);
	
	return m_pipelineState.create(desc, &m_pipelineState, device);
}

void RenderPassVoxelMip::createViews(ID3D12Device* device)
{
	auto voxelTextureDiffuse = s_resourceManager->getTexture(L"voxelTextureDiffuse");
	auto texDesc = voxelTextureDiffuse->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R32_UINT;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
	srvDesc.Texture3D.MipLevels = 1;
	srvDesc.Texture3D.MostDetailedMip = 0;
	srvDesc.Texture3D.ResourceMinLODClamp = 0;

	auto handle = m_uavHeap.getHandleCpu();
	device->CreateShaderResourceView(voxelTextureDiffuse, &srvDesc, handle);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_R32_UINT;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	uavDesc.Texture3D.FirstWSlice = 0;
	uavDesc.Texture3D.MipSlice = 1;
	uavDesc.Texture3D.WSize = texDesc.DepthOrArraySize / 2;

	handle.offset(1);
	device->CreateUnorderedAccessView(voxelTextureDiffuse, nullptr, &uavDesc, handle);
}

bool RenderPassVoxelMip::initialize(ID3D12Device* device, void* p)
{
	if (!loadShaders())
		return false;

	if (!createRootSignature(device))
		return false;

	if (!createPipelineState(device))
		return false;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	heapDesc.NodeMask = 1;
	heapDesc.NumDescriptors = 2;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (!m_uavHeap.create(heapDesc, device))
	{
		return false;
	}

	if (!m_commandList.create(device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAlloc, m_pipelineState.get()))
		return false;

	createViews(device);

	return true;
}

void RenderPassVoxelMip::shutdown()
{

}

void RenderPassVoxelMip::submitCommands(ID3D12CommandList** list, u32* index)
{
	const u32 voxelDim = 64;
	const u32 voxelDimZ = voxelDim * 6;

	auto voxelTextureDiffuse = s_resourceManager->getTexture(L"voxelTextureDiffuse");

	m_commandList->Reset(m_cmdAlloc, m_pipelineState.get());

	u32 srcMip = 0;
	u32 dstMip = 1;

	u32 dstDim = voxelDim >> dstMip;
	u32 dstDimZ = voxelDimZ >> dstMip;

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(voxelTextureDiffuse, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, srcMip));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(voxelTextureDiffuse));

	auto heap = m_uavHeap.get();
	m_commandList->SetDescriptorHeaps(1, &heap);
	m_commandList->SetGraphicsRootSignature(m_rootSignature.get());
	m_commandList->SetGraphicsRootDescriptorTable(0, m_uavHeap->GetGPUDescriptorHandleForHeapStart());

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	m_commandList->DrawInstanced(dstDim, dstDim * dstDimZ, 0, 0);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(voxelTextureDiffuse));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(voxelTextureDiffuse, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, srcMip));

	m_commandList->Close();

	list[*index] = m_commandList.get();
	++(*index);
}