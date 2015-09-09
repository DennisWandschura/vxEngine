#include "RenderPassDrawVoxel.h"
#include "ResourceManager.h"
#include "ShaderManager.h"
#include "d3dx12.h"
#include "GpuVoxel.h"

RenderPassDrawVoxel::RenderPassDrawVoxel(ID3D12CommandAllocator* cmdAlloc)
	:m_cmdAlloc(cmdAlloc)
{

}

RenderPassDrawVoxel::~RenderPassDrawVoxel()
{

}

void RenderPassDrawVoxel::getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device)
{
	D3D12_RESOURCE_DESC resDesc[2];
	resDesc[0].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc[0].Alignment = 64 KBYTE;
	resDesc[0].Width = s_resolution.x;
	resDesc[0].Height = s_resolution.y;
	resDesc[0].DepthOrArraySize = 1;
	resDesc[0].MipLevels = 1;
	resDesc[0].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc[0].SampleDesc.Count = 1;
	resDesc[0].SampleDesc.Quality = 0;
	resDesc[0].Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc[0].Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	resDesc[1].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc[1].Alignment = 64 KBYTE;
	resDesc[1].Width = s_resolution.x;
	resDesc[1].Height = s_resolution.y;
	resDesc[1].DepthOrArraySize = 1;
	resDesc[1].MipLevels = 1;
	resDesc[1].Format = DXGI_FORMAT_D32_FLOAT;
	resDesc[1].SampleDesc.Count = 1;
	resDesc[1].SampleDesc.Quality = 0;
	resDesc[1].Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc[1].Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	auto info = device->GetResourceAllocationInfo(1, 2, resDesc);

	*heapSizeRtDs += info.SizeInBytes;
}

bool RenderPassDrawVoxel::createData(ID3D12Device* device)
{
	D3D12_RESOURCE_DESC resDesc[2];
	resDesc[0].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc[0].Alignment = 64 KBYTE;
	resDesc[0].Width = s_resolution.x;
	resDesc[0].Height = s_resolution.y;
	resDesc[0].DepthOrArraySize = 1;
	resDesc[0].MipLevels = 1;
	resDesc[0].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc[0].SampleDesc.Count = 1;
	resDesc[0].SampleDesc.Quality = 0;
	resDesc[0].Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc[0].Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	resDesc[1].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc[1].Alignment = 64 KBYTE;
	resDesc[1].Width = s_resolution.x;
	resDesc[1].Height = s_resolution.y;
	resDesc[1].DepthOrArraySize = 1;
	resDesc[1].MipLevels = 1;
	resDesc[1].Format = DXGI_FORMAT_D32_FLOAT;
	resDesc[1].SampleDesc.Count = 1;
	resDesc[1].SampleDesc.Quality = 0;
	resDesc[1].Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc[1].Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	auto info = device->GetResourceAllocationInfo(1, 1, &resDesc[0]);

	D3D12_CLEAR_VALUE clearValue = 
	{
		DXGI_FORMAT_R8G8B8A8_UNORM,
		{0, 0, 0, 0}
	};

	CreateResourceDesc desc;
	desc.clearValue = &clearValue;
	desc.resDesc = &resDesc[0];
	desc.size = info.SizeInBytes;
	desc.state = D3D12_RESOURCE_STATE_RENDER_TARGET;

	if (s_resourceManager->createTextureRtDs(L"voxelDebug", desc) == nullptr)
		return false;

	auto infoDepth = device->GetResourceAllocationInfo(1, 1, &resDesc[1]);

	D3D12_CLEAR_VALUE clearValueDepth;
	clearValueDepth.Format = DXGI_FORMAT_D32_FLOAT;
	clearValueDepth.DepthStencil.Depth = 1;
	clearValueDepth.DepthStencil.Stencil = 0;

	desc.clearValue = &clearValueDepth;
	desc.resDesc = &resDesc[1];
	desc.size = infoDepth.SizeInBytes;
	desc.state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	if (s_resourceManager->createTextureRtDs(L"voxelDebugDepth", desc) == nullptr)
		return false;

	return true;
}

bool RenderPassDrawVoxel::loadShaders()
{
	if (!s_shaderManager->loadShader("DrawVoxelVS.cso", L"../../lib/DrawVoxelVS.cso", d3d::ShaderType::Vertex))
		return false;

	if (!s_shaderManager->loadShader("DrawVoxelGS.cso", L"../../lib/DrawVoxelGS.cso", d3d::ShaderType::Geometry))
		return false;

	if (!s_shaderManager->loadShader("DrawVoxelPS.cso", L"../../lib/DrawVoxelPS.cso", d3d::ShaderType::Pixel))
		return false;

	return true;
}

bool RenderPassDrawVoxel::createRootSignature(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangeVS[1];
	rangeVS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 0);

	CD3DX12_DESCRIPTOR_RANGE rangeGS[2];
	rangeGS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0, 0, 0);
	rangeGS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, 2);

	CD3DX12_ROOT_PARAMETER rootParameters[2];
	rootParameters[0].InitAsDescriptorTable(1, rangeVS, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsDescriptorTable(2, rangeGS, D3D12_SHADER_VISIBILITY_GEOMETRY);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(2, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

bool RenderPassDrawVoxel::createPipelineState(ID3D12Device* device)
{
	auto rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	d3d::PipelineStateDescInput inputDesc;
	inputDesc.depthEnabled = 1;
	inputDesc.rootSignature = m_rootSignature.get();
	inputDesc.shaderDesc.vs = s_shaderManager->getShader("DrawVoxelVS.cso");
	inputDesc.shaderDesc.gs = s_shaderManager->getShader("DrawVoxelGS.cso");
	inputDesc.shaderDesc.ps = s_shaderManager->getShader("DrawVoxelPS.cso");
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	inputDesc.dsvFormat = DXGI_FORMAT_D32_FLOAT;
	inputDesc.rtvFormats = &rtvFormat;
	inputDesc.rtvCount = 1;

	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);
	//desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	return d3d::PipelineState::create(desc, &m_pipelineState, device);
}

bool RenderPassDrawVoxel::createDescriptorHeap(ID3D12Device* device)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 3;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (!m_descriptorHeapSrv.create(desc, device))
		return false;

	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	if (device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_heapRtv.getAddressOf())) != 0)
		return false;

	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	if (device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_heapDsv.getAddressOf())) != 0)
		return false;

	return true;
}

void RenderPassDrawVoxel::createViews(ID3D12Device* device)
{
	auto voxelBuffer = s_resourceManager->getBuffer(L"voxelBuffer");
	auto voxelTextureOpacity = s_resourceManager->getTexture(L"voxelTextureOpacity");

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDescVoxel;
	cbvDescVoxel.BufferLocation = voxelBuffer->GetGPUVirtualAddress();
	cbvDescVoxel.SizeInBytes = d3d::AlignedSizeType<GpuVoxel, 1, 256>::size;

	auto cbvCamera = s_resourceManager->getConstantBufferView("cameraBufferView");

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R32_UINT;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
	srvDesc.Texture3D.MipLevels = 1;
	srvDesc.Texture3D.MostDetailedMip = 0;
	srvDesc.Texture3D.ResourceMinLODClamp = 0;

	auto handle = m_descriptorHeapSrv.getHandleCpu();
	device->CreateConstantBufferView(&cbvDescVoxel, handle);

	handle.offset(1);
	device->CreateConstantBufferView(cbvCamera, handle);

	handle.offset(1);
	device->CreateShaderResourceView(voxelTextureOpacity->get(), &srvDesc, handle);
}

void RenderPassDrawVoxel::createDepthRenderTarget(ID3D12Device* device)
{
	auto voxelDebug = s_resourceManager->getTextureRtDs(L"voxelDebug");
	auto voxelDebugDepth = s_resourceManager->getTextureRtDs(L"voxelDebugDepth");

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.Texture2D.MipSlice = 0;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	device->CreateRenderTargetView(voxelDebug->get(), &rtvDesc, m_heapRtv->GetCPUDescriptorHandleForHeapStart());

	device->CreateDepthStencilView(voxelDebugDepth->get(), &dsvDesc, m_heapDsv->GetCPUDescriptorHandleForHeapStart());
}

bool RenderPassDrawVoxel::initialize(ID3D12Device* device, void* p)
{
	if (!loadShaders())
		return false;

	if (!createRootSignature(device))
		return false;

	if (!createPipelineState(device))
		return false;

	if (!createDescriptorHeap(device))
		return false;

	if (!m_commandList.create(device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAlloc, m_pipelineState.get()))
		return false;

	createViews(device);

	createDepthRenderTarget(device);

	return true;
}

void RenderPassDrawVoxel::shutdown()
{

}

void RenderPassDrawVoxel::submitCommands(Graphics::CommandQueue* queue)
{
	auto voxelBuffer = s_resourceManager->getBuffer(L"voxelBuffer");
	auto voxelTextureOpacity = s_resourceManager->getTexture(L"voxelTextureOpacity");

	const f32 clearColor[] = { 0, 0, 0, 0 };

	auto rtvHandle = m_heapRtv->GetCPUDescriptorHandleForHeapStart();
	auto dsvHandle = m_heapDsv->GetCPUDescriptorHandleForHeapStart();

	D3D12_VIEWPORT viewPort;
	viewPort.Height = (f32)s_resolution.y;
	viewPort.Width = (f32)s_resolution.x;
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;

	D3D12_RECT rectScissor;
	rectScissor.left = 0;
	rectScissor.top = 0;
	rectScissor.right = s_resolution.x;
	rectScissor.bottom = s_resolution.y;

	m_commandList->Reset(m_cmdAlloc, m_pipelineState.get());

	m_commandList->RSSetViewports(1, &viewPort);
	m_commandList->RSSetScissorRects(1, &rectScissor);

	m_commandList->OMSetRenderTargets(1, &rtvHandle, 0, &dsvHandle);

	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0 , nullptr);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(voxelBuffer->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(voxelTextureOpacity->get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

	auto srvHeap = m_descriptorHeapSrv.get();
	m_commandList->SetDescriptorHeaps(1, &srvHeap);

	m_commandList->SetGraphicsRootSignature(m_rootSignature.get());
	m_commandList->SetGraphicsRootDescriptorTable(0, srvHeap->GetGPUDescriptorHandleForHeapStart());
	m_commandList->SetGraphicsRootDescriptorTable(1, srvHeap->GetGPUDescriptorHandleForHeapStart());

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	m_commandList->DrawInstanced(256, 256 * 256, 0, 0);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(voxelTextureOpacity->get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(voxelBuffer->get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	m_commandList->Close();

	queue->pushCommandList(&m_commandList);
}