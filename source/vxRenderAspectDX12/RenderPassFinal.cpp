#include "RenderPassFinal.h"
#include <d3d12.h>
#include "ShaderManager.h"
#include "d3dx12.h"
#include "ResourceManager.h"
#include "Device.h"

RenderPassFinal::RenderPassFinal(ID3D12CommandAllocator* cmdAlloc, d3d::Device* device)
	:RenderPass(),
	m_commandList(),
	m_cmdAlloc(cmdAlloc),
	m_descriptorHeapSrv(),
	m_descriptorHeapRtv(),
	m_device(device),
	m_renderTarget(),
	m_resolution(0, 0)
{

}

RenderPassFinal::~RenderPassFinal()
{

}

void RenderPassFinal::getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device)
{

}

bool RenderPassFinal::loadShaders(d3d::ShaderManager* shaderManager)
{
	if (!shaderManager->loadShader("DrawQuadVs.cso", L"../../lib/DrawQuadVs.cso", d3d::ShaderType::Vertex))
		return false;

	if (!shaderManager->loadShader("DrawQuadGs.cso", L"../../lib/DrawQuadGs.cso", d3d::ShaderType::Geometry))
		return false;

	if (!shaderManager->loadShader("FinalPS.cso", L"../../lib/FinalPS.cso", d3d::ShaderType::Pixel))
		return false;

	return true;
}

bool RenderPassFinal::createRootSignature(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangePS[1];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0, 0);

	CD3DX12_ROOT_PARAMETER rootParameters[1];
	rootParameters[0].InitAsDescriptorTable(1, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = 6.0f;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(1, rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

bool RenderPassFinal::createPipelineState(ID3D12Device* device, d3d::ShaderManager* shaderManager)
{
	auto vsShader = shaderManager->getShader("DrawQuadVs.cso");
	auto gsShader = shaderManager->getShader("DrawQuadGs.cso");
	auto psShader = shaderManager->getShader("FinalPS.cso");

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
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
	psoDesc.SampleDesc.Count = 1;

	auto hresult = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipelineState.getAddressOf()));
	if (hresult != 0)
		return false;

	return true;
}

bool RenderPassFinal::initialize(ID3D12Device* device, void* p)
{
	auto diffuseTexture = s_resourceManager->getTextureRtDs(L"diffuseTexture");
	auto aoTexture = s_resourceManager->getTextureRtDs(L"aoTexture");

	if (!loadShaders(s_shaderManager))
		return false;

	if (!createRootSignature(device))
		return false;

	if (!createPipelineState(device, s_shaderManager))
		return false;

	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 2;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (!m_descriptorHeapSrv.create(desc, device))
		return false;

	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 2;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	if (!m_descriptorHeapRtv.create(desc, device))
		return false;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0;

	auto handle = m_descriptorHeapSrv.getHandleCpu();
	device->CreateShaderResourceView(aoTexture, &srvDesc, handle);

	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	handle.offset(1);
	device->CreateShaderResourceView(diffuseTexture, &srvDesc, handle);

	auto rtvHandle = m_descriptorHeapRtv.getHandleCpu();
	for (u32 i = 0; i < 2; ++i)
	{
		m_device->getBuffer(i, IID_PPV_ARGS(m_renderTarget[i].getAddressOf()));

		m_renderTarget[i]->SetName(L"DeviceRenderTarget");

		device->CreateRenderTargetView(m_renderTarget[i].get(), nullptr, rtvHandle);
		rtvHandle.offset(1);
	}

	auto rtDesc = m_renderTarget[0]->GetDesc();
	m_resolution.x = rtDesc.Width;
	m_resolution.y = rtDesc.Height;

	if (device->CreateCommandList(1, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAlloc, m_pipelineState.get(), IID_PPV_ARGS(m_commandList.getAddressOf())) != 0)
		return false;
	m_commandList->Close();

	return true;
}

void RenderPassFinal::shutdown()
{
	m_commandList.destroy();
	m_cmdAlloc = nullptr;
	m_descriptorHeapRtv.destroy();
	m_descriptorHeapSrv.destroy();
}

ID3D12CommandList* RenderPassFinal::submitCommands()
{
	auto aoTexture = s_resourceManager->getTextureRtDs(L"aoTexture");
	auto diffuseTexture = s_resourceManager->getTextureRtDs(L"diffuseTexture");
	auto currentBuffer = m_device->getCurrentBackBufferIndex();

	D3D12_VIEWPORT viewport;
	viewport.Height = (f32)m_resolution.y;
	viewport.Width = (f32)m_resolution.x;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	D3D12_RECT rectScissor;
	rectScissor.left = 0;
	rectScissor.top = 0;
	rectScissor.right = m_resolution.x;
	rectScissor.bottom = m_resolution.y;

	const f32 clearColor[] = { 0.10f, 0.22f, 0.5f, 1 };
	auto rtvHandle = m_descriptorHeapRtv.getHandleCpu();
	rtvHandle.offset(currentBuffer);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[1];
	rtvHandles[0] = rtvHandle;

	auto hresult = m_commandList->Reset(m_cmdAlloc, m_pipelineState.get());
	VX_ASSERT(hresult == 0);

	m_commandList->RSSetViewports(1, &viewport);
	m_commandList->RSSetScissorRects(1, &rectScissor);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(aoTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(diffuseTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTarget[currentBuffer].get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	m_commandList->SetGraphicsRootSignature(m_rootSignature.get());

	ID3D12DescriptorHeap* heaps[]=
	{
		m_descriptorHeapSrv.get()
	};

	m_commandList->SetDescriptorHeaps(1, heaps);
	m_commandList->SetGraphicsRootDescriptorTable(0, m_descriptorHeapSrv->GetGPUDescriptorHandleForHeapStart());

	m_commandList->OMSetRenderTargets(1, rtvHandles, FALSE, nullptr);
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	m_commandList->DrawInstanced(1, 1, 0, 0);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTarget[currentBuffer].get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(diffuseTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(aoTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));

	hresult = m_commandList->Close();
	VX_ASSERT(hresult == 0);

	return m_commandList.get();
}