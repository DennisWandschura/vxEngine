#include "RenderPassZBufferCreateMipmaps.h"
#include "ResourceManager.h"
#include "ShaderManager.h"
#include "d3dx12.h"
#include "CommandAllocator.h"
#include "CommandList.h"

RenderPassZBufferCreateMipmaps::RenderPassZBufferCreateMipmaps()
{

}

RenderPassZBufferCreateMipmaps::~RenderPassZBufferCreateMipmaps()
{
}

void RenderPassZBufferCreateMipmaps::getRequiredMemory(u64* heapSizeBuffer, u32* bufferCount, u64* heapSizeTexture, u32* textureCount, u64* heapSizeRtDs, u32* rtDsCount, ID3D12Device* device)
{

}

bool RenderPassZBufferCreateMipmaps::createData(ID3D12Device* device)
{
	return true;
}

bool RenderPassZBufferCreateMipmaps::loadShaders(d3d::ShaderManager* shaderManager)
{
	if (!shaderManager->loadShader(L"SAO_minify.cso"))
		return false;

	return true;
}

bool RenderPassZBufferCreateMipmaps::createRootSignature(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangePS[1];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, 0);

	CD3DX12_ROOT_PARAMETER rootParameters[1];
	rootParameters[0].InitAsDescriptorTable(1, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(1, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	return m_rootSignature.create(&rootSignatureDesc, device);
}

bool RenderPassZBufferCreateMipmaps::createPipelineState(ID3D12Device* device, d3d::ShaderManager* shaderManager)
{
	auto rtvFormat = DXGI_FORMAT_R32_FLOAT;

	d3d::PipelineStateDescInput inputDesc;
	inputDesc.rootSignature = m_rootSignature.get();
	inputDesc.depthEnabled = 0;
	inputDesc.shaderDesc.vs = shaderManager->getShader(L"DrawQuadVs.cso");
	inputDesc.shaderDesc.gs = shaderManager->getShader(L"DrawQuadGs.cso");
	inputDesc.shaderDesc.ps = shaderManager->getShader(L"SAO_minify.cso");
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	inputDesc.rtvFormats = &rtvFormat;
	inputDesc.rtvCount = 1;
	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);

	return d3d::PipelineState::create(desc, &m_pipelineState, device);

	/*D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { nullptr, 0 };
	psoDesc.pRootSignature = m_rootSignature.get();
	psoDesc.VS = { reinterpret_cast<UINT8*>(vsShader->GetBufferPointer()), vsShader->GetBufferSize() };
	psoDesc.GS = { reinterpret_cast<UINT8*>(gsShader->GetBufferPointer()), gsShader->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<UINT8*>(psShader->GetBufferPointer()), psShader->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.FrontCounterClockwise = 1;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = 0;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R32_FLOAT;
	psoDesc.SampleDesc.Count = 1;

	auto hresult = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipelineState.getAddressOf()));
	if (hresult != 0)
		return false;*/

	return true;
}

bool RenderPassZBufferCreateMipmaps::createViews(ID3D12Device* device, d3d::DescriptorHandleCpu* rtvHandle, d3d::DescriptorHandleCpu* srvHandle, ID3D12Resource* texture)
{
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.PlaneSlice = 0;

	for (u32 i = 1; i <= 5; ++i)
	{
		rtvDesc.Texture2D.MipSlice = i;

		device->CreateRenderTargetView(texture, &rtvDesc, *rtvHandle);

		rtvHandle->offset(1);
	}

	for (u32 i = 0; i < 5; ++i)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = i;
		srvDesc.Texture2D.PlaneSlice = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0;
		device->CreateShaderResourceView(texture, &srvDesc, *srvHandle);

		srvHandle->offset(1);
	}

	return true;
}

bool RenderPassZBufferCreateMipmaps::initialize(ID3D12Device* device, d3d::CommandAllocator* allocators, u32 frameCount)
{
	if (!loadShaders(s_shaderManager))
		return false;

	if (!createRootSignature(device))
		return false;

	if (!createPipelineState(device, s_shaderManager))
		return false;

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 1;
	rtvHeapDesc.NumDescriptors = 10;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	if (!m_descriptorHeapRtv.create(rtvHeapDesc, device))
		return false;

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvHeapDesc.NodeMask = 1;
	srvHeapDesc.NumDescriptors = 12;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (!m_descriptorHeapSrv.create(srvHeapDesc, device))
		return false;

	auto srvHandle = m_descriptorHeapSrv.getHandleCpu();
	auto rtvHandle = m_descriptorHeapRtv.getHandleCpu();

	auto zBuffer = s_resourceManager->getTextureRtDs(L"zBuffer");
	if (!createViews(device, &rtvHandle, &srvHandle, zBuffer->get()))
		return false;

	if (!createCommandLists(device, D3D12_COMMAND_LIST_TYPE_DIRECT, allocators, frameCount))
	{
		return false;
	}

	return true;
}

void RenderPassZBufferCreateMipmaps::shutdown()
{

}

void RenderPassZBufferCreateMipmaps::createMipMaps(ID3D12Resource* texture, d3d::DescriptorHandleCpu* rtvHandleCpu, d3d::DescriptorHandleGpu* srvHandleGpu)
{
	const f32 clearColor[4] = { 1.0f, 0.0f, 0, 0 };

	D3D12_VIEWPORT viewport;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	D3D12_RECT rectScissor;
	rectScissor.left = 0;
	rectScissor.top = 0;

	auto x = s_resolution.x >> 1;
	auto y = s_resolution.y >> 1;

	auto commandList = m_currentCommandList->get();

	//auto rtvHandle = rtvHeap->getHandleCpu();
	for (u32 i = 1; i <= 5; ++i)
	{
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET, i));

		viewport.Height = (f32)y;
		viewport.Width = (f32)x;
		rectScissor.right = x;
		rectScissor.bottom = y;

		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &rectScissor);

		D3D12_CPU_DESCRIPTOR_HANDLE descHandle = *rtvHandleCpu;
		commandList->OMSetRenderTargets(1, &descHandle, 0, nullptr);

		commandList->ClearRenderTargetView(descHandle, clearColor, 0, nullptr);

		commandList->SetGraphicsRootDescriptorTable(0, *srvHandleGpu);

		commandList->DrawInstanced(1, 1, 0, 0);

		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ, i));

		x = x >> 1;
		y = y >> 1;

		rtvHandleCpu->offset(1);
		srvHandleGpu->offset(1);
	}
}

void RenderPassZBufferCreateMipmaps::buildCommands(d3d::CommandAllocator* currentAllocator, u32 frameIndex)
{
	auto zBuffer = s_resourceManager->getTextureRtDs(L"zBuffer");

	auto &currentCommandList = m_commandLists[frameIndex];
	m_currentCommandList = &currentCommandList;

	currentCommandList->Reset(currentAllocator->get(), m_pipelineState.get());

	currentCommandList->SetPipelineState(m_pipelineState.get());
	currentCommandList->SetGraphicsRootSignature(m_rootSignature.get());

	currentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	auto heap = m_descriptorHeapSrv.get();
	currentCommandList->SetDescriptorHeaps(1, &heap);

	auto srvHandleGpu = m_descriptorHeapSrv.getHandleGpu();
	auto rtvHandleCpu = m_descriptorHeapRtv.getHandleCpu();

	createMipMaps(zBuffer->get(), &rtvHandleCpu, &srvHandleGpu);

	auto hr = currentCommandList->Close();
}

void RenderPassZBufferCreateMipmaps::submitCommands(Graphics::CommandQueue* queue)
{
	queue->pushCommandList(m_currentCommandList);
}