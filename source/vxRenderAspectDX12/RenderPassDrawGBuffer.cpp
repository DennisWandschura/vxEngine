#include "RenderPassDrawGBuffer.h"
#include <d3d12.h>
#include "ShaderManager.h"
#include "d3dx12.h"
#include "ResourceManager.h"

RenderPassDrawGBuffer::RenderPassDrawGBuffer(ID3D12CommandAllocator* cmdAlloc)
	:RenderPass(),
	m_commandList(),
	m_cmdAlloc(cmdAlloc),
	m_descriptorHeap()
{

}

RenderPassDrawGBuffer::~RenderPassDrawGBuffer()
{

}

void RenderPassDrawGBuffer::getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device)
{

}

bool RenderPassDrawGBuffer::loadShaders(d3d::ShaderManager* shaderManager)
{
	if (!shaderManager->loadShader("DrawQuadVs.cso", L"../../lib/DrawQuadVs.cso", d3d::ShaderType::Vertex))
		return false;

	if (!shaderManager->loadShader("DrawQuadGs.cso", L"../../lib/DrawQuadGs.cso", d3d::ShaderType::Geometry))
		return false;

	if (!shaderManager->loadShader("DrawQuadPs.cso", L"../../lib/DrawQuadPs.cso", d3d::ShaderType::Pixel))
		return false;

	return true;
}

bool RenderPassDrawGBuffer::createRootSignature(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangePS[1];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_ROOT_PARAMETER rootParameters[1];
	rootParameters[0].InitAsDescriptorTable(1, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = 0.0f;
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

bool RenderPassDrawGBuffer::createPipelineState(ID3D12Device* device, d3d::ShaderManager* shaderManager)
{
	auto vsShader = shaderManager->getShader("DrawQuadVs.cso");
	auto gsShader = shaderManager->getShader("DrawQuadGs.cso");
	auto psShader = shaderManager->getShader("DrawQuadPs.cso");

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

bool RenderPassDrawGBuffer::initialize(ID3D12Device* device, void* p)
{
	auto gbufferDiffuse = s_resourceManager->getTexture(L"gbufferDiffuse");

	if (!loadShaders(s_shaderManager))
		return false;

	if (!createRootSignature(device))
		return false;

	if (!createPipelineState(device, s_shaderManager))
		return false;

	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (!m_descriptorHeap.create(desc, device))
		return false;

	D3D12_SHADER_RESOURCE_VIEW_DESC zbufferDesc;
	zbufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	zbufferDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	zbufferDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	zbufferDesc.Texture2DArray.ArraySize = 2;
	zbufferDesc.Texture2DArray.FirstArraySlice = 0;
	zbufferDesc.Texture2DArray.MipLevels = 1;
	zbufferDesc.Texture2DArray.MostDetailedMip = 0;
	zbufferDesc.Texture2DArray.PlaneSlice = 0;
	zbufferDesc.Texture2DArray.ResourceMinLODClamp = 0;

	device->CreateShaderResourceView(gbufferDiffuse, &zbufferDesc, m_descriptorHeap.getHandleCpu());

	return true;
}

void RenderPassDrawGBuffer::shutdown()
{
	m_commandList.destroy();
	m_cmdAlloc = nullptr;
	m_descriptorHeap.destroy();
}

ID3D12CommandList* RenderPassDrawGBuffer::submitCommands()
{
	auto gbufferDiffuse = s_resourceManager->getTexture(L"gbufferDiffuse");

	m_commandList->Reset(m_cmdAlloc, m_pipelineState.get());

	//m_commandList->RSSetViewports(1, &m_viewport);
	//m_commandList->RSSetScissorRects(1, &m_rectScissor);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gbufferDiffuse, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	m_commandList->SetGraphicsRootSignature(m_rootSignature.get());

	ID3D12DescriptorHeap* heaps[]=
	{
		m_descriptorHeap.get()
	};

	m_commandList->SetDescriptorHeaps(1, heaps);
	m_commandList->SetGraphicsRootDescriptorTable(0, m_descriptorHeap->GetGPUDescriptorHandleForHeapStart());

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	m_commandList->DrawInstanced(1, 1, 0, 0);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gbufferDiffuse, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));

	m_commandList->Close();

	return m_commandList.get();
}