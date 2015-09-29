#include "RenderPassVoxelPropagate.h"
#include "ShaderManager.h"
#include "ResourceManager.h"
#include "CommandAllocator.h"
#include "GpuProfiler.h"

RenderPassVoxelPropagate::RenderPassVoxelPropagate(d3d::CommandAllocator* cmdAlloc)
	:m_commandList(),
	m_cmdAlloc(cmdAlloc),
	m_srvHeap()
{

}

RenderPassVoxelPropagate::~RenderPassVoxelPropagate()
{

}

void RenderPassVoxelPropagate::getRequiredMemory(u64* heapSizeBuffer, u32* bufferCount, u64* heapSizeTexture, u32* textureCount, u64* heapSizeRtDs, u32* rtDsCount, ID3D12Device* device)
{

}

bool RenderPassVoxelPropagate::createData(ID3D12Device* device)
{
	return true;
}

bool RenderPassVoxelPropagate::loadShaders()
{
	if (!s_shaderManager->loadShader(L"VoxelPropagateVS.cso"))
		return false;

	return true;
}

bool RenderPassVoxelPropagate::createRootSignature(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangeVS[2];
	rangeVS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, 0);
	rangeVS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, 1);

	CD3DX12_ROOT_PARAMETER rootParameters[2];
	rootParameters[0].InitAsDescriptorTable(2, rangeVS, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsConstants(1, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(2, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	return m_rootSignature.create(&rootSignatureDesc, device);
}

bool RenderPassVoxelPropagate::createPipelineState(ID3D12Device* device)
{
	d3d::PipelineStateDescInput inputDesc;
	inputDesc.rootSignature = m_rootSignature.get();
	inputDesc.depthEnabled = 0;
	inputDesc.shaderDesc.vs = s_shaderManager->getShader(L"VoxelPropagateVS.cso");
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);

	return d3d::PipelineState::create(desc, &m_pipelineState, device);
}

bool RenderPassVoxelPropagate::createDescriptorHeap(ID3D12Device* device)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 4;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	return m_srvHeap.create(desc, device);
}

void RenderPassVoxelPropagate::createViews(ID3D12Device* device)
{
	auto voxelTextureColorTmp = s_resourceManager->getTexture(L"voxelTextureColorTmp");
	auto voxelTextureColor = s_resourceManager->getTexture(L"voxelTextureColor");

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Texture3D.MipLevels = 1;
	srvDesc.Texture3D.MostDetailedMip = 0;
	srvDesc.Texture3D.ResourceMinLODClamp = 0;

	auto handle = m_srvHeap.getHandleCpu();
	device->CreateShaderResourceView(voxelTextureColorTmp->get(), &srvDesc, handle);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	uavDesc.Texture3D.FirstWSlice = 0;
	uavDesc.Texture3D.MipSlice = 0;
	uavDesc.Texture3D.WSize = s_settings->m_lpvDim * 6;
	handle.offset(1);
	device->CreateUnorderedAccessView(voxelTextureColor->get(), nullptr, &uavDesc, handle);

	handle.offset(1);
	device->CreateShaderResourceView(voxelTextureColor->get(), &srvDesc, handle);

	handle.offset(1);
	device->CreateUnorderedAccessView(voxelTextureColorTmp->get(), nullptr, &uavDesc, handle);
}

bool RenderPassVoxelPropagate::initialize(ID3D12Device* device, void* p)
{
	if (!loadShaders())
		return false;

	if (!createRootSignature(device))
		return false;

	if (!createPipelineState(device))
		return false;

	if (!createDescriptorHeap(device))
		return false;

	if (!m_commandList.create(device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAlloc->get(), m_pipelineState.get()))
		return false;

	createViews(device);

	return true;
}

void RenderPassVoxelPropagate::shutdown()
{

}

void RenderPassVoxelPropagate::buildCommands()
{
	auto dim = s_settings->m_lpvDim;

	m_commandList->Reset(m_cmdAlloc->get(), m_pipelineState.get());

	s_gpuProfiler->queryBegin("propagate", &m_commandList);

	auto srvHeap = m_srvHeap.get();
	m_commandList->SetDescriptorHeaps(1, &srvHeap);
	m_commandList->SetGraphicsRootSignature(m_rootSignature.get());

	auto gpuHandle = m_srvHeap.getHandleGpu();

	auto gpuHandle0 = gpuHandle;
	gpuHandle.offset(2);
	auto gpuHandle1 = gpuHandle;

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	for (u32 i = 0; i < 3; ++i)
	{
		m_commandList->SetGraphicsRootDescriptorTable(0, gpuHandle1);
		for (u32 axis = 0; axis < 6; ++axis)
		{
			m_commandList->SetGraphicsRoot32BitConstant(1, axis, 0);
			m_commandList->DrawInstanced(dim, dim* dim, 0, 0);
		}

		m_commandList->SetGraphicsRootDescriptorTable(0, gpuHandle0);
		for (u32 axis = 0; axis < 6; ++axis)
		{
			m_commandList->SetGraphicsRoot32BitConstant(1, axis, 0);
			m_commandList->DrawInstanced(dim, dim* dim, 0, 0);
		}
	}

	s_gpuProfiler->queryEnd(&m_commandList);

	m_commandList->Close();
}

void RenderPassVoxelPropagate::submitCommands(Graphics::CommandQueue* queue)
{
	queue->pushCommandList(&m_commandList);
}