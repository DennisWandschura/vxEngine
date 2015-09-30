#include "RenderPassCreateVpl.h"
#include "CommandAllocator.h"
#include "ResourceManager.h"
#include "ShaderManager.h"
#include "GpuProfiler.h"

RenderPassCreateVpl::RenderPassCreateVpl(d3d::CommandAllocator* allocator)
	:m_commandList(),
	m_allocator(allocator),
	m_srvHeap()
{

}

RenderPassCreateVpl::~RenderPassCreateVpl()
{

}

void RenderPassCreateVpl::getRequiredMemory(u64* heapSizeBuffer, u32* bufferCount, u64* heapSizeTexture, u32* textureCount, u64* heapSizeRtDs, u32* rtDsCount, ID3D12Device* device)
{

}

bool RenderPassCreateVpl::createData(ID3D12Device* device)
{
	return true;
}

bool RenderPassCreateVpl::createRootSignature(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangeVS[2];
	rangeVS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0, 0);
	rangeVS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 3, 0, 0, 2);

	CD3DX12_ROOT_PARAMETER rootParameters[1];
	rootParameters[0].InitAsDescriptorTable(2, rangeVS, D3D12_SHADER_VISIBILITY_VERTEX);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(1, rootParameters, 0, 0, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	return m_rootSignature.create(&rootSignatureDesc, device);
}

bool RenderPassCreateVpl::createPipelineStage(ID3D12Device* device)
{
	d3d::PipelineStateDescInput inputDesc;
	inputDesc.rootSignature = m_rootSignature.get();
	inputDesc.depthEnabled = 0;
	inputDesc.shaderDesc.vs = s_shaderManager->getShader(L"CreateVplVS.cso");
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);

	return d3d::PipelineState::create(desc, &m_pipelineState, device);
}

bool RenderPassCreateVpl::createDescriptorHeap(ID3D12Device* device)
{
	/*
	Texture3D<float4> g_colorTexture : register(t0);
Texture3D<float4> g_normalTexture : register(t1);

RWTexture3D<float4> g_lpvRed : register(u0);
RWTexture3D<float4> g_lpvGreen : register(u1);
RWTexture3D<float4> g_lpvBlue : register(u2);
*/

	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 5;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	if(!m_srvHeap.create(desc, device))
		return false;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Shader4ComponentMapping =D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING ;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
	srvDesc.Texture3D.MipLevels = 1;
	srvDesc.Texture3D.MostDetailedMip = 0;
	srvDesc.Texture3D.ResourceMinLODClamp = 0;

	auto voxelTextureColor = s_resourceManager->getTexture(L"voxelTextureColor");
	auto handle = m_srvHeap.getHandleCpu();
	device->CreateShaderResourceView(voxelTextureColor->get(), &srvDesc, handle);

	auto voxelTextureNormals = s_resourceManager->getTexture(L"voxelTextureNormals");
	handle.offset(1);
	device->CreateShaderResourceView(voxelTextureNormals->get(), &srvDesc, handle);

	auto lpvTextureRed = s_resourceManager->getTexture(L"lpvTextureRed");
	auto lpvTextureGreen = s_resourceManager->getTexture(L"lpvTextureGreen");
	auto lpvTextureBlue = s_resourceManager->getTexture(L"lpvTextureBlue");

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	uavDesc.Texture3D.FirstWSlice = 0;
	uavDesc.Texture3D.MipSlice = 0;
	uavDesc.Texture3D.WSize = s_settings->m_lpvDim;

	handle.offset(1);
	device->CreateUnorderedAccessView(lpvTextureRed->get(), nullptr, &uavDesc, handle);

	handle.offset(1);
	device->CreateUnorderedAccessView(lpvTextureGreen->get(), nullptr, &uavDesc, handle);

	handle.offset(1);
	device->CreateUnorderedAccessView(lpvTextureBlue->get(), nullptr, &uavDesc, handle);

	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NumDescriptors = 3;
	if (!m_uavHeap.create(desc, device))
			return false;

	handle = m_uavHeap.getHandleCpu();
	device->CreateUnorderedAccessView(lpvTextureRed->get(), nullptr, &uavDesc, handle);
	handle.offset(1);
	device->CreateUnorderedAccessView(lpvTextureGreen->get(), nullptr, &uavDesc, handle);

	handle.offset(1);
	device->CreateUnorderedAccessView(lpvTextureBlue->get(), nullptr, &uavDesc, handle);

	return true;
}

bool RenderPassCreateVpl::initialize(ID3D12Device* device, void* p)
{
	const wchar_t* shader = L"CreateVplVS.cso";
	if (!loadShaders(&shader, 1))
		return false;

	if (!createRootSignature(device))
		return false;

	if (!createPipelineStage(device))
		return false;

	if (!createDescriptorHeap(device))
		return false;

	if (!m_commandList.create(device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_allocator->get(), m_pipelineState.get()))
		return false;

	return true;
}

void RenderPassCreateVpl::shutdown()
{

}

void RenderPassCreateVpl::buildCommands()
{
	const f32 clearColor[4] = {0, 0, 0, 0};
	auto dim = s_settings->m_lpvDim;

	m_commandList->Reset(m_allocator->get(), m_pipelineState.get());

	auto lpvTextureRed = s_resourceManager->getTexture(L"lpvTextureRed");
	auto lpvTextureGreen = s_resourceManager->getTexture(L"lpvTextureGreen");
	auto lpvTextureBlue = s_resourceManager->getTexture(L"lpvTextureBlue");

	auto voxelTextureColor = s_resourceManager->getTexture(L"voxelTextureColor");
	auto voxelTextureNormals = s_resourceManager->getTexture(L"voxelTextureNormals");

	s_gpuProfiler->queryBegin("create vpl", &m_commandList);

	auto cpuHandle = m_uavHeap.getHandleCpu();
	auto gpuHandle = m_uavHeap.getHandleGpu();
	m_commandList->ClearUnorderedAccessViewFloat(gpuHandle, cpuHandle, lpvTextureRed->get(), clearColor, 0, 0);

	cpuHandle.offset(1);
	gpuHandle.offset(1);
	m_commandList->ClearUnorderedAccessViewFloat(gpuHandle, cpuHandle, lpvTextureGreen->get(), clearColor, 0, 0);

	cpuHandle.offset(1);
	gpuHandle.offset(1);
	m_commandList->ClearUnorderedAccessViewFloat(gpuHandle, cpuHandle, lpvTextureBlue->get(), clearColor, 0, 0);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(lpvTextureRed->get()));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(lpvTextureGreen->get()));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(lpvTextureBlue->get()));

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(voxelTextureColor->get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(voxelTextureNormals->get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

	auto descriptorHeap = m_srvHeap.get();

	m_commandList->SetDescriptorHeaps(1, &descriptorHeap);
	m_commandList->SetGraphicsRootSignature(m_rootSignature.get());
	m_commandList->SetGraphicsRootDescriptorTable(0, descriptorHeap->GetGPUDescriptorHandleForHeapStart());

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	m_commandList->DrawInstanced(dim, dim * dim, 0, 0);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(voxelTextureNormals->get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(voxelTextureColor->get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(lpvTextureRed->get()));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(lpvTextureGreen->get()));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(lpvTextureBlue->get()));

	s_gpuProfiler->queryEnd(&m_commandList);

	m_commandList->Close();
}

void RenderPassCreateVpl::submitCommands(Graphics::CommandQueue* queue)
{
	queue->pushCommandList(&m_commandList);
}