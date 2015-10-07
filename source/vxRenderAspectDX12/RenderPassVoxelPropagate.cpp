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
	rangeVS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 0, 0);
	rangeVS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 3, 0, 0, 3);

	CD3DX12_ROOT_PARAMETER rootParameters[1];
	rootParameters[0].InitAsDescriptorTable(2, rangeVS, D3D12_SHADER_VISIBILITY_VERTEX);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(1, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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
	desc.NumDescriptors = 6 * 2;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	return m_srvHeap.create(desc, device);
}

void RenderPassVoxelPropagate::createViews(ID3D12Device* device)
{
	auto lpvTextureRed = s_resourceManager->getTexture(L"lpvTextureRed");
	auto lpvTextureGreen = s_resourceManager->getTexture(L"lpvTextureGreen");
	auto lpvTextureBlue = s_resourceManager->getTexture(L"lpvTextureBlue");

	auto lpvTextureRedTmp = s_resourceManager->getTexture(L"lpvTextureRedTmp");
	auto lpvTextureGreenTmp = s_resourceManager->getTexture(L"lpvTextureGreenTmp");
	auto lpvTextureBlueTmp = s_resourceManager->getTexture(L"lpvTextureBlueTmp");

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvDesc.Texture3D.MipLevels = 1;
	srvDesc.Texture3D.MostDetailedMip = 0;
	srvDesc.Texture3D.ResourceMinLODClamp = 0;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	uavDesc.Texture3D.FirstWSlice = 0;
	uavDesc.Texture3D.MipSlice = 0;
	uavDesc.Texture3D.WSize = s_settings->m_lpvDim;

	auto handle = m_srvHeap.getHandleCpu();
	{
		device->CreateShaderResourceView(lpvTextureRed->get(), &srvDesc, handle);

		handle.offset(1);
		device->CreateShaderResourceView(lpvTextureGreen->get(), &srvDesc, handle);

		handle.offset(1);
		device->CreateShaderResourceView(lpvTextureBlue->get(), &srvDesc, handle);

		handle.offset(1);
		device->CreateUnorderedAccessView(lpvTextureRedTmp->get(), nullptr, &uavDesc, handle);

		handle.offset(1);
		device->CreateUnorderedAccessView(lpvTextureGreenTmp->get(), nullptr, &uavDesc, handle);

		handle.offset(1);
		device->CreateUnorderedAccessView(lpvTextureBlueTmp->get(), nullptr, &uavDesc, handle);
	}

	{
		handle.offset(1);
		device->CreateShaderResourceView(lpvTextureRedTmp->get(), &srvDesc, handle);

		handle.offset(1);
		device->CreateShaderResourceView(lpvTextureGreenTmp->get(), &srvDesc, handle);

		handle.offset(1);
		device->CreateShaderResourceView(lpvTextureBlueTmp->get(), &srvDesc, handle);

		handle.offset(1);
		device->CreateUnorderedAccessView(lpvTextureRed->get(), nullptr, &uavDesc, handle);

		handle.offset(1);
		device->CreateUnorderedAccessView(lpvTextureGreen->get(), nullptr, &uavDesc, handle);

		handle.offset(1);
		device->CreateUnorderedAccessView(lpvTextureBlue->get(), nullptr, &uavDesc, handle);
	}
	
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
	gpuHandle.offset(6);
	auto gpuHandle1 = gpuHandle;

	auto lpvTextureRed = s_resourceManager->getTexture(L"lpvTextureRed");
	auto lpvTextureGreen = s_resourceManager->getTexture(L"lpvTextureGreen");
	auto lpvTextureBlue = s_resourceManager->getTexture(L"lpvTextureBlue");

	auto lpvTextureRedTmp = s_resourceManager->getTexture(L"lpvTextureRedTmp");
	auto lpvTextureGreenTmp = s_resourceManager->getTexture(L"lpvTextureGreenTmp");
	auto lpvTextureBlueTmp = s_resourceManager->getTexture(L"lpvTextureBlueTmp");

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	for (u32 i = 0; i < 3; ++i)
	{
		lpvTextureRed->barrierTransition(m_commandList.get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		lpvTextureGreen->barrierTransition(m_commandList.get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		lpvTextureBlue->barrierTransition(m_commandList.get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		lpvTextureRedTmp->barrierTransition(m_commandList.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		lpvTextureGreenTmp->barrierTransition(m_commandList.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		lpvTextureBlueTmp->barrierTransition(m_commandList.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		m_commandList->SetGraphicsRootDescriptorTable(0, gpuHandle0);
		m_commandList->DrawInstanced(dim * dim, dim, 0, 0);

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(lpvTextureRedTmp->get()));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(lpvTextureGreenTmp->get()));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(lpvTextureBlueTmp->get()));

		lpvTextureRed->barrierTransition(m_commandList.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		lpvTextureGreen->barrierTransition(m_commandList.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		lpvTextureBlue->barrierTransition(m_commandList.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		lpvTextureRedTmp->barrierTransition(m_commandList.get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		lpvTextureGreenTmp->barrierTransition(m_commandList.get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		lpvTextureBlueTmp->barrierTransition(m_commandList.get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		m_commandList->SetGraphicsRootDescriptorTable(0, gpuHandle1);
		m_commandList->DrawInstanced(dim * dim, dim, 0, 0);

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(lpvTextureRed->get()));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(lpvTextureGreen->get()));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(lpvTextureBlue->get()));
	}

	s_gpuProfiler->queryEnd(&m_commandList);

	m_commandList->Close();
}

void RenderPassVoxelPropagate::submitCommands(Graphics::CommandQueue* queue)
{
	queue->pushCommandList(&m_commandList);
}