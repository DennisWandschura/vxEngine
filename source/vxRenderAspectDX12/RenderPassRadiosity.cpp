#include "RenderPassRadiosity.h"
#include "ResourceManager.h"
#include "d3dx12.h"
#include "ShaderManager.h"
#include "GpuSaoBuffer.h"
#include "CommandAllocator.h"

RenderPassRadiosity::RenderPassRadiosity(d3d::CommandAllocator* alloc)
	:m_allocator(alloc),
	m_commandList()
{

}

RenderPassRadiosity::~RenderPassRadiosity()
{

}

void RenderPassRadiosity::getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device)
{
	D3D12_RESOURCE_DESC resDesc[2];
	resDesc[0].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc[0].Alignment = 64 KBYTE;
	resDesc[0].Width = s_resolution.x;
	resDesc[0].Height = s_resolution.y;
	resDesc[0].DepthOrArraySize = 1;
	resDesc[0].MipLevels = 1;
	resDesc[0].Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	resDesc[0].SampleDesc.Count = 1;
	resDesc[0].SampleDesc.Quality = 0;
	resDesc[0].Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc[0].Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	resDesc[1] = resDesc[0];
	// indirect0
	// indirect1

	auto info =  device->GetResourceAllocationInfo(1, 2, resDesc);
	*heapSizeRtDs += info.SizeInBytes;
}

bool RenderPassRadiosity::createData(ID3D12Device* device)
{
	D3D12_RESOURCE_DESC resDesc[1];
	resDesc[0].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc[0].Alignment = 64 KBYTE;
	resDesc[0].Width = s_resolution.x;
	resDesc[0].Height = s_resolution.y;
	resDesc[0].DepthOrArraySize = 1;
	resDesc[0].MipLevels = 1;
	resDesc[0].Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	resDesc[0].SampleDesc.Count = 1;
	resDesc[0].SampleDesc.Quality = 0;
	resDesc[0].Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc[0].Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	clearValue.Color[0] = 0;
	clearValue.Color[1] = 0;
	clearValue.Color[2] = 0;
	clearValue.Color[3] = 0;

	auto info = device->GetResourceAllocationInfo(1, 1, resDesc);

	CreateResourceDesc desc;
	desc.clearValue = &clearValue;
	desc.resDesc = resDesc;
	desc.size = info.SizeInBytes;
	desc.state = D3D12_RESOURCE_STATE_RENDER_TARGET;
	auto p = s_resourceManager->createTextureRtDs(L"bounce0", desc);
	if (p == nullptr)
		return false;

	clearValue.Color[0] = 0;

	p = s_resourceManager->createTextureRtDs(L"bounce1", desc);
	if (p == nullptr)
		return false;

	return true;
}

bool RenderPassRadiosity::loadShaders()
{
	if (!s_shaderManager->loadShader("RadiosityPS.cso", L"../../lib/RadiosityPS.cso", d3d::ShaderType::Pixel))
		return false;

	if (!s_shaderManager->loadShader("RadiosityBouncePS.cso", L"../../lib/RadiosityBouncePS.cso", d3d::ShaderType::Pixel))
		return false;

	return true;
}

bool RenderPassRadiosity::createRootSignature(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangePS[2];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 0, 0);
	rangePS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 3);

	CD3DX12_ROOT_PARAMETER rootParameters[1];
	rootParameters[0].InitAsDescriptorTable(2, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(1, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	return m_rootSignature.create(&rootSignatureDesc, device);
}

bool RenderPassRadiosity::createRootSignatureGather(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangePS[1];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 0, 4);

	CD3DX12_ROOT_PARAMETER rootParameters[1];
	rootParameters[0].InitAsDescriptorTable(1, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(1, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob* blob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	auto hresult = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &errorBlob);
	if (hresult != 0)
		return false;

	hresult = device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(m_rootSignatureGather.getAddressOf()));
	if (hresult != 0)
		return false;

	return true;
}

bool RenderPassRadiosity::createPipelineState(ID3D12Device* device)
{
	auto rtvFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

	d3d::PipelineStateDescInput inputDesc;
	inputDesc.rootSignature = m_rootSignature.get();
	inputDesc.depthEnabled = 0;
	inputDesc.shaderDesc.vs = s_shaderManager->getShader("DrawQuadVs.cso");
	inputDesc.shaderDesc.gs = s_shaderManager->getShader("DrawQuadGs.cso");
	inputDesc.shaderDesc.ps = s_shaderManager->getShader("RadiosityPS.cso");
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	inputDesc.rtvFormats = &rtvFormat;
	inputDesc.rtvCount = 1;
	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);

	return d3d::PipelineState::create(desc, &m_pipelineState, device);
}

bool RenderPassRadiosity::createPipelineStateGather(ID3D12Device* device)
{
	auto rtvFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

	d3d::PipelineStateDescInput inputDesc;
	inputDesc.rootSignature = m_rootSignatureGather.get();
	inputDesc.depthEnabled = 0;
	inputDesc.shaderDesc.vs = s_shaderManager->getShader("DrawQuadVs.cso");
	inputDesc.shaderDesc.gs = s_shaderManager->getShader("DrawQuadGs.cso");
	inputDesc.shaderDesc.ps = s_shaderManager->getShader("RadiosityBouncePS.cso");
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	inputDesc.rtvFormats = &rtvFormat;
	inputDesc.rtvCount = 1;
	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);

	return d3d::PipelineState::create(desc, &m_pipelineStateGather, device);
}

bool RenderPassRadiosity::createRtv(ID3D12Device* device)
{
	auto bounce0 = s_resourceManager->getTextureRtDs(L"bounce0");
	auto bounce1 = s_resourceManager->getTextureRtDs(L"bounce1");

	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 2;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	if (!m_rtvHeap.create(desc, device))
		return false;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	auto handle = m_rtvHeap.getHandleCpu();
	device->CreateRenderTargetView(bounce0->get(), &rtvDesc, handle);

	handle.offset(1);
	device->CreateRenderTargetView(bounce1->get(), &rtvDesc, handle);

	return true;
}

bool RenderPassRadiosity::createSrv(ID3D12Device* device)
{ /*
  Texture2D CS_Z_buffer: register(t0); 0
Texture2D g_bounceTexture: register(t1); 1
Texture2DArray g_normalTexture : register(t2); 2
  cbuffer SaoBufferBlock : register(b0) 3

  Texture2D indirectBuffer : register(t0); 4
  Texture2D directBuffer : register(t1); 5
  Texture2DArray albedoBuffer : register(t2); 6
  */

	auto zBuffer0 = s_resourceManager->getTextureRtDs(L"zBuffer0");
	auto bounce1 = s_resourceManager->getTextureRtDs(L"bounce1");
	auto gbufferNormal = s_resourceManager->getTextureRtDs(L"gbufferNormal");
	auto saoBuffer = s_resourceManager->getBuffer(L"saoBuffer");

	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 7;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (!m_srvHeap.create(desc, device))
		return false;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 6;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0;

	auto handle = m_srvHeap.getHandleCpu();
	device->CreateShaderResourceView(zBuffer0->get(), &srvDesc, handle);

	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvDesc.Texture2D.MipLevels = 1;
	handle.offset(1);
	device->CreateShaderResourceView(bounce1->get(), &srvDesc, handle);

	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
	srvDesc.Texture2DArray.ArraySize = 1;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.MipLevels = 1;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.PlaneSlice = 0;
	srvDesc.Texture2DArray.ResourceMinLODClamp=0;
	handle.offset(1);
	device->CreateShaderResourceView(gbufferNormal->get(), &srvDesc, handle);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = saoBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = d3d::AlignedSizeType<GpuSaoBuffer, 1, 256>::size;
	handle.offset(1);
	device->CreateConstantBufferView(&cbvDesc, handle);

	///////////////////////////////////////////7
	auto bounce0 = s_resourceManager->getTextureRtDs(L"bounce0");
	auto directLightTexture = s_resourceManager->getTextureRtDs(L"directLightTexture");
	auto gbufferAlbedo = s_resourceManager->getTextureRtDs(L"gbufferAlbedo");

	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0;

	handle.offset(1);
	device->CreateShaderResourceView(bounce0->get(), &srvDesc, handle);

	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0;
	handle.offset(1);
	device->CreateShaderResourceView(directLightTexture->get(), &srvDesc, handle);

	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Texture2DArray.ArraySize = 1;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.MipLevels = 1;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.PlaneSlice = 0;
	srvDesc.Texture2DArray.ResourceMinLODClamp = 0;
	handle.offset(1);
	device->CreateShaderResourceView(gbufferAlbedo->get(), &srvDesc, handle);

	return true;
}

bool RenderPassRadiosity::createCommandList(ID3D12Device* device)
{
	return m_commandList.create(device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_allocator->get(), m_pipelineState.get());
}

bool RenderPassRadiosity::initialize(ID3D12Device* device, void* p)
{
	if (!loadShaders())
		return false;

	if (!createRootSignature(device))
		return false;

	if (!createRootSignatureGather(device))
		return false;

	if (!createPipelineState(device))
		return false;

	if (!createPipelineStateGather(device))
		return false;

	if (!createRtv(device))
		return false;

	if (!createSrv(device))
		return false;

	if (!createCommandList(device))
		return false;

	return true;
}

void RenderPassRadiosity::shutdown()
{
	m_pipelineState.destroy();
	m_rootSignature.destroy();
	m_commandList.destroy();
}

void RenderPassRadiosity::submitCommands(Graphics::CommandQueue* queue)
{
	const f32 clearcolor[4] = {0, 0, 0, 0};
	auto bounce0 = s_resourceManager->getTextureRtDs(L"bounce0");
	auto bounce1 = s_resourceManager->getTextureRtDs(L"bounce1");

	m_commandList->Reset(m_allocator->get(), m_pipelineState.get());

	D3D12_VIEWPORT viewport;
	viewport.Height = (f32)s_resolution.y;
	viewport.Width = (f32)s_resolution.x;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	D3D12_RECT rectScissor;
	rectScissor.left = 0;
	rectScissor.top = 0;
	rectScissor.right = s_resolution.x;
	rectScissor.bottom = s_resolution.y;

	m_commandList->RSSetViewports(1, &viewport);
	m_commandList->RSSetScissorRects(1, &rectScissor);

	/*CD3DX12_TEXTURE_COPY_LOCATION src(bounce1, 0);
	CD3DX12_TEXTURE_COPY_LOCATION dst(bounce0, 0);

	// copy previous final radiosity to src
	CD3DX12_RESOURCE_BARRIER barriers[2];
	barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(bounce0, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST, 0);
	barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(bounce1, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE, 0);
	m_commandList->ResourceBarrier(2, barriers);
	m_commandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(bounce0, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET, 0);
	barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(bounce1, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0);
	m_commandList->ResourceBarrier(2, barriers);*/

	auto zBuffer0 = s_resourceManager->getTextureRtDs(L"zBuffer0");
	auto gbufferNormal = s_resourceManager->getTextureRtDs(L"gbufferNormal");

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	ID3D12DescriptorHeap* srvHeaps[] =
	{
		m_srvHeap.get()
	};

	m_commandList->SetDescriptorHeaps(1, srvHeaps);

	auto rtvHandle = m_rtvHeap.getHandleCpu();
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvHandle = rtvHandle;
	m_commandList->OMSetRenderTargets(1, &d3dRtvHandle, 0, nullptr);
	m_commandList->ClearRenderTargetView(rtvHandle, clearcolor, 0, nullptr);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(bounce1->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(zBuffer0->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gbufferNormal->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0));

	m_commandList->SetGraphicsRootSignature(m_rootSignature.get());
	m_commandList->SetGraphicsRootDescriptorTable(0, m_srvHeap->GetGPUDescriptorHandleForHeapStart());

	m_commandList->DrawInstanced(1, 1, 0, 0);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gbufferNormal->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(zBuffer0->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(bounce1->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0));

	////////////////////////////////////////
	auto directLightTexture = s_resourceManager->getTextureRtDs(L"directLightTexture");
	auto gbufferAlbedo = s_resourceManager->getTextureRtDs(L"gbufferAlbedo");

	rtvHandle.offset(1);
	d3dRtvHandle = rtvHandle;
	m_commandList->SetPipelineState(m_pipelineStateGather.get());

	m_commandList->OMSetRenderTargets(1, &d3dRtvHandle, 0, nullptr);
	m_commandList->ClearRenderTargetView(rtvHandle, clearcolor, 0, nullptr);

	m_commandList->SetGraphicsRootSignature(m_rootSignatureGather.get());
	m_commandList->SetGraphicsRootDescriptorTable(0, m_srvHeap->GetGPUDescriptorHandleForHeapStart());

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(bounce0->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gbufferAlbedo->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(directLightTexture->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0));

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	m_commandList->DrawInstanced(1, 1, 0, 0);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(directLightTexture->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gbufferAlbedo->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(bounce0->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0));

	m_commandList->Close();

	queue->pushCommandList(&m_commandList);
}