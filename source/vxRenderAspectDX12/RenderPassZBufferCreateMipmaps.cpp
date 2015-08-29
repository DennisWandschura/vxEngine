#include "RenderPassZBufferCreateMipmaps.h"
#include <d3d12.h>
#include "ResourceManager.h"
#include "ShaderManager.h"
#include "d3dx12.h"

RenderPassZBufferCreateMipmaps::RenderPassZBufferCreateMipmaps(ID3D12CommandAllocator* cmdAlloc)
	:m_commandList(),
	m_cmdAlloc(cmdAlloc)
{

}

RenderPassZBufferCreateMipmaps::~RenderPassZBufferCreateMipmaps()
{
}

void RenderPassZBufferCreateMipmaps::getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device)
{

}

bool RenderPassZBufferCreateMipmaps::loadShaders(d3d::ShaderManager* shaderManager)
{
	if (!shaderManager->loadShader("SAO_minify.cso", L"../../lib/SAO_minify.cso", d3d::ShaderType::Vertex))
		return false;

	return true;
}

bool RenderPassZBufferCreateMipmaps::createRootSignature(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangePS[1];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_ROOT_PARAMETER rootParameters[1];
	rootParameters[0].InitAsDescriptorTable(1, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);

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

bool RenderPassZBufferCreateMipmaps::createPipelineState(ID3D12Device* device, d3d::ShaderManager* shaderManager)
{
	auto vsShader = shaderManager->getShader("DrawQuadVs.cso");
	auto gsShader = shaderManager->getShader("DrawQuadGs.cso");
	auto psShader = shaderManager->getShader("SAO_minify.cso");

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
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
		return false;

	return true;
}

bool RenderPassZBufferCreateMipmaps::initialize(ID3D12Device* device, void* p)
{
	if (!loadShaders(s_shaderManager))
		return false;

	if (!createRootSignature(device))
		return false;

	if (!createPipelineState(device, s_shaderManager))
		return false;

	auto zBuffer = s_resourceManager->getTextureRtDs(L"zBuffer");

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 1;
	rtvHeapDesc.NumDescriptors = 5;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	if(!m_descriptorHeapRtv.create(rtvHeapDesc, device))
		return false;

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvHeapDesc.NodeMask = 1;
	srvHeapDesc.NumDescriptors = 6;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (!m_descriptorHeapSrv.create(srvHeapDesc, device))
		return false;

	/*D3D12_SHADER_RESOURCE_VIEW_DESC zbufferDesc;
	zbufferDesc.Format = DXGI_FORMAT_R32_FLOAT;
	zbufferDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	zbufferDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	zbufferDesc.Texture2DArray.ArraySize = 2;
	zbufferDesc.Texture2DArray.FirstArraySlice = 0;
	zbufferDesc.Texture2DArray.MipLevels = 1;
	zbufferDesc.Texture2DArray.MostDetailedMip = 0;
	zbufferDesc.Texture2DArray.PlaneSlice = 0;
	zbufferDesc.Texture2DArray.ResourceMinLODClamp = 0;*/
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.PlaneSlice = 0;

	auto rtvHandle = m_descriptorHeapRtv.getHandleCpu();
	for (u32 i = 1; i <= 5; ++i)
	{
		rtvDesc.Texture2D.MipSlice = i;

		device->CreateRenderTargetView(zBuffer, &rtvDesc, rtvHandle);

		rtvHandle.offset(1);
	}

	m_zbuffer = zBuffer;

	auto texDesc = m_zbuffer->GetDesc();
	m_textureResolution.x = texDesc.Width;
	m_textureResolution.y = texDesc.Height;

	auto srvHandle = m_descriptorHeapSrv.getHandleCpu();
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
		device->CreateShaderResourceView(zBuffer, &srvDesc, srvHandle);

		srvHandle.offset(1);
	}

	if (!m_commandList.create(device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAlloc, m_pipelineState.get()))
		return false;

	return true;
}

void RenderPassZBufferCreateMipmaps::shutdown()
{

}

ID3D12CommandList* RenderPassZBufferCreateMipmaps::submitCommands()
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

	m_commandList->Reset(m_cmdAlloc, m_pipelineState.get());

	m_commandList->SetPipelineState(m_pipelineState.get());
	m_commandList->SetGraphicsRootSignature(m_rootSignature.get());

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	//

	auto x = m_textureResolution.x >> 1;
	auto y = m_textureResolution.y >> 1;
	auto rtvHandle = m_descriptorHeapRtv.getHandleCpu();
	auto srvHandle = m_descriptorHeapSrv.getHandleCpu();

	u32 srcMip = 0;
	for (u32 i = 1; i <= 5; ++i)
	{
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_zbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, srcMip));

		viewport.Height = (f32)y;
		viewport.Width = (f32)x;
		rectScissor.right = x;
		rectScissor.bottom = y;

		m_commandList->RSSetViewports(1, &viewport);
		m_commandList->RSSetScissorRects(1, &rectScissor);

		D3D12_CPU_DESCRIPTOR_HANDLE descHandle = rtvHandle;
		m_commandList->OMSetRenderTargets(1, &descHandle, 0, nullptr);

		m_commandList->ClearRenderTargetView(descHandle, clearColor, 0, nullptr);

		auto heap = m_descriptorHeapSrv.get();
		m_commandList->SetDescriptorHeaps(1, &heap);
		m_commandList->SetGraphicsRootDescriptorTable(0, m_descriptorHeapSrv->GetGPUDescriptorHandleForHeapStart());

		m_commandList->DrawInstanced(1, 1, 0, 0);

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_zbuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, srcMip));

		x = x >> 1;
		y = y >> 1;
		++srcMip;

		rtvHandle.offset(1);
		srvHandle.offset(1);
	}

	m_commandList->Close();
	
	return m_commandList.get();;
}