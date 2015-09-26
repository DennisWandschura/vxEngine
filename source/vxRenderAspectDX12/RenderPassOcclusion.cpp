#include "RenderPassOcclusion.h"
#include "ShaderManager.h"
#include "ResourceManager.h"
#include "CommandAllocator.h"
#include "GpuVoxel.h"

RenderPassOcclusion::RenderPassOcclusion(d3d::CommandAllocator* allocator)
	:m_commandList(),
	m_allocator(allocator),
	m_rtvHeap(),
	m_srvHeap()
{

}

RenderPassOcclusion::~RenderPassOcclusion()
{

}

void RenderPassOcclusion::getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device)
{
	D3D12_RESOURCE_DESC resDesc;
	resDesc.Alignment = 64 KBYTE;
	resDesc.DepthOrArraySize = 1;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Height = s_resolution.y;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.MipLevels = 3;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Width = s_resolution.x;

	auto alloc = device->GetResourceAllocationInfo(1, 1, &resDesc);

	*heapSizeRtDs += alloc.SizeInBytes;
}

bool RenderPassOcclusion::createData(ID3D12Device* device)
{
	D3D12_RESOURCE_DESC resDesc;
	resDesc.Alignment = 64 KBYTE;
	resDesc.DepthOrArraySize = 1;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Height = s_resolution.y;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Width = s_resolution.x;

	auto alloc = device->GetResourceAllocationInfo(1, 1, &resDesc);

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	CreateResourceDesc desc;
	desc.clearValue = &clearValue;
	desc.resDesc = &resDesc;
	desc.size = alloc.SizeInBytes;
	desc.state = D3D12_RESOURCE_STATE_RENDER_TARGET;

	auto voxelOcclusion = s_resourceManager->createTextureRtDs(L"voxelOcclusion", desc);
	if (voxelOcclusion == nullptr)
		return false;

	return true;
}

bool RenderPassOcclusion::loadShaders()
{
	if (!s_shaderManager->loadShader(L"ConeTraceOpacityVS.cso"))
		return false;

	if (!s_shaderManager->loadShader(L"ConeTraceOpacityPS.cso"))
		return false;

	return true;
}

bool RenderPassOcclusion::createRootSignature(ID3D12Device* device)
{
	/*
	cbuffer VoxelBuffer : register(b0)
cbuffer CameraBuffer : register(b1)
cbuffer CameraStaticBuffer : register(b2)

Texture2D<float> g_zBuffer : register(t0);
Texture2DArray<half2> g_normalSlice : register(t1);

cbuffer VoxelBuffer : register(b0)
Texture3D<uint> g_opacity : register(t2);
*/
	CD3DX12_DESCRIPTOR_RANGE rangeVS[2];
	rangeVS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 3, 0, 0, 0);
	rangeVS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0, 3);

	CD3DX12_DESCRIPTOR_RANGE rangePS[2];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 0);
	rangePS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0, 5);

	CD3DX12_ROOT_PARAMETER rootParameters[2];
	rootParameters[0].InitAsDescriptorTable(2, rangeVS, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsDescriptorTable(2, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(2, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	return m_rootSignature.create(&rootSignatureDesc, device);
}

bool RenderPassOcclusion::createPipelineState(ID3D12Device* device)
{
	auto rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	d3d::PipelineStateDescInput inputDesc;
	inputDesc.depthEnabled = 0;
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	inputDesc.rootSignature = m_rootSignature.get();
	inputDesc.shaderDesc.vs = s_shaderManager->getShader(L"ConeTraceOpacityVS.cso");
	inputDesc.shaderDesc.ps = s_shaderManager->getShader(L"ConeTraceOpacityPS.cso");
	inputDesc.rtvCount = 1;
	inputDesc.rtvFormats = &rtvFormat;

	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);

	return d3d::PipelineState::create(desc, &m_pipelineState, device);
}

bool RenderPassOcclusion::createRtv(ID3D12Device* device)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 3;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	if(!m_rtvHeap.create(desc, device))
		return false;

	auto voxelOcclusion = s_resourceManager->getTextureRtDs(L"voxelOcclusion");

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	auto handle = m_rtvHeap.getHandleCpu();
	device->CreateRenderTargetView(voxelOcclusion->get(),&rtvDesc, handle);

	rtvDesc.Texture2D.MipSlice = 1;
	//device->CreateRenderTargetView(voxelOcclusion->get(), &rtvDesc, handle);

	//rtvDesc.Texture2D.MipSlice = 2;
	//device->CreateRenderTargetView(voxelOcclusion->get(), &rtvDesc, handle);

	return true;
}

bool RenderPassOcclusion::createSrv(ID3D12Device* device)
{
	/*
	cbuffer VoxelBuffer : register(b0)
	cbuffer CameraBuffer : register(b1)
	cbuffer CameraStaticBuffer : register(b2)

	Texture2D<float> g_zBuffer : register(t0);
	Texture2DArray<half2> g_normalSlice : register(t1);
	Texture3D<uint> g_opacity : register(t2);
	*/

	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 6;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	if(!m_srvHeap.create(desc, device))
		return false;

	auto voxelBuffer = s_resourceManager->getBuffer(L"voxelBuffer");

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = voxelBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = d3d::AlignedSizeType<GpuVoxel, 1, 256>::size;

	auto handle = m_srvHeap.getHandleCpu();
	device->CreateConstantBufferView(&cbvDesc, handle);

	auto cameraBufferViewDesc = s_resourceManager->getConstantBufferView("cameraBufferView");
	handle.offset(1);
	device->CreateConstantBufferView(cameraBufferViewDesc, handle);

	auto cameraStaticBufferView = s_resourceManager->getConstantBufferView("cameraStaticBufferView");
	handle.offset(1);
	device->CreateConstantBufferView(cameraStaticBufferView, handle);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescZBuffer;
	srvDescZBuffer.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescZBuffer.Format = DXGI_FORMAT_R32_FLOAT;
	srvDescZBuffer.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDescZBuffer.Texture2D.MipLevels = 1;
	srvDescZBuffer.Texture2D.MostDetailedMip = 0;
	srvDescZBuffer.Texture2D.PlaneSlice = 0;
	srvDescZBuffer.Texture2D.ResourceMinLODClamp = 0;
	handle.offset(1);
	auto zBuffer = s_resourceManager->getTextureRtDs(L"zBuffer0");
	device->CreateShaderResourceView(zBuffer->get(), &srvDescZBuffer, handle);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescNormal;
	srvDescNormal.Format = DXGI_FORMAT_R16G16_FLOAT;
	srvDescNormal.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescNormal.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDescNormal.Texture2DArray.ArraySize = 1;
	srvDescNormal.Texture2DArray.FirstArraySlice = 0;
	srvDescNormal.Texture2DArray.MipLevels = 1;
	srvDescNormal.Texture2DArray.MostDetailedMip = 0;
	srvDescNormal.Texture2DArray.PlaneSlice = 0;
	srvDescNormal.Texture2DArray.ResourceMinLODClamp = 0;

	auto gbufferNormal = s_resourceManager->getTextureRtDs(L"gbufferNormal");
	handle.offset(1);
	device->CreateShaderResourceView(gbufferNormal->get(), &srvDescNormal, handle);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescVoxel;
	srvDescVoxel.Format = DXGI_FORMAT_R32_UINT;
	srvDescVoxel.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescVoxel.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
	srvDescVoxel.Texture3D.MipLevels = 4;
	srvDescVoxel.Texture3D.MostDetailedMip = 0;
	srvDescVoxel.Texture3D.ResourceMinLODClamp = 0;

	auto voxelTextureOpacity = s_resourceManager->getTexture(L"voxelTextureOpacity");
	handle.offset(1);
	device->CreateShaderResourceView(voxelTextureOpacity->get(), &srvDescVoxel, handle);

	return true;
}

bool RenderPassOcclusion::initialize(ID3D12Device* device, void* p)
{
	if (!loadShaders())
		return false;

	if (!createRootSignature(device))
		return false;

	if (!createPipelineState(device))
		return false;

	if(!m_commandList.create(device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_allocator->get(), m_pipelineState.get()))
		return false;

	if (!createRtv(device))
		return false;

	if (!createSrv(device))
		return false;

	return true;
}

void RenderPassOcclusion::shutdown()
{

}

void RenderPassOcclusion::submitCommands(Graphics::CommandQueue* queue)
{
	auto voxelTextureOpacity = s_resourceManager->getTexture(L"voxelTextureOpacity");

	const f32 clearColor[4] = { 0, 0, 0, 0 };
	m_commandList->Reset(m_allocator->get(), m_pipelineState.get());

	auto resolution = s_resolution;

	D3D12_VIEWPORT viewport;
	viewport.Height = (f32)resolution.y;
	viewport.Width = (f32)resolution.x;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	D3D12_RECT rectScissor;
	rectScissor.left = 0;
	rectScissor.top = 0;
	rectScissor.right = resolution.x;
	rectScissor.bottom = resolution.y;

	m_commandList->RSSetViewports(1, &viewport);
	m_commandList->RSSetScissorRects(1, &rectScissor);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(voxelTextureOpacity->get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap.getHandleCpu();

	m_commandList->OMSetRenderTargets(1, &rtvHandle, 0, nullptr);
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	auto heap = m_srvHeap.get();
	m_commandList->SetDescriptorHeaps(1, &heap);
	m_commandList->SetGraphicsRootSignature(m_rootSignature.get());
	m_commandList->SetGraphicsRootDescriptorTable(0, heap->GetGPUDescriptorHandleForHeapStart());
	m_commandList->SetGraphicsRootDescriptorTable(1, heap->GetGPUDescriptorHandleForHeapStart());

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	m_commandList->DrawInstanced(resolution.x, resolution.y, 0, 0);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(voxelTextureOpacity->get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	/*auto voxelOcclusion = s_resourceManager->getTextureRtDs(L"voxelOcclusion");

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(voxelOcclusion->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, 0));

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(voxelOcclusion->get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0));*/

	m_commandList->Close();

	queue->pushCommandList(&m_commandList);
}