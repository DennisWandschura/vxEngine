/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "GBufferRenderer.h"
#include <d3d12.h>
#include "d3dx12.h"
#include "ShaderManager.h"
#include "Device.h"

#include "d3dx12.h"

GBufferRenderer::GBufferRenderer()
{

}

GBufferRenderer::~GBufferRenderer()
{

}

bool GBufferRenderer::loadShaders(ShaderManager* shaderManager)
{
	if (!shaderManager->loadShader("MeshVertex.cso", L"../../lib/MeshVertex.cso", ShaderType::Vertex))
		return false;

	if (!shaderManager->loadShader("DeepGBufferGs.cso", L"../../lib/DeepGBufferGs.cso", ShaderType::Geometry))
		return false;

	if (!shaderManager->loadShader("PsGBuffer.cso", L"../../lib/PsGBuffer.cso", ShaderType::Pixel))
		return false;

	return true;
}

bool GBufferRenderer::createRootSignature(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangesVS[3];
	rangesVS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 0);
	rangesVS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0, 1);

	CD3DX12_DESCRIPTOR_RANGE rangesGS[1];
	rangesGS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 0);

	CD3DX12_DESCRIPTOR_RANGE rangePS[1];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 2, 0, 3);

	CD3DX12_ROOT_PARAMETER rootParameters[3];
	rootParameters[0].InitAsDescriptorTable(2, rangesVS, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsDescriptorTable(1, rangesGS, D3D12_SHADER_VISIBILITY_GEOMETRY);
	rootParameters[2].InitAsDescriptorTable(1, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

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
	rootSignatureDesc.Init(3, rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

bool GBufferRenderer::createPipelineState(ID3D12Device* device, ShaderManager* shaderManager)
{
	/*
	float3 position : POSITION0;
	float3 normal : NORMAL0;
	float3 tangent : TANGENT0;
	float2 uv : TEXCOORD0;
	uint drawId : BLENDINDICES0;
	*/
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32_UINT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 }
	};

	auto vsShader = shaderManager->getShader("MeshVertex.cso");
	auto gsShader = shaderManager->getShader("DeepGBufferGs.cso");
	auto psShader = shaderManager->getShader("PsGBuffer.cso");

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
	psoDesc.pRootSignature = m_rootSignature.get();
	psoDesc.VS = { reinterpret_cast<UINT8*>(vsShader->GetBufferPointer()), vsShader->GetBufferSize() };
	psoDesc.GS = { reinterpret_cast<UINT8*>(gsShader->GetBufferPointer()), gsShader->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<UINT8*>(psShader->GetBufferPointer()), psShader->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.FrontCounterClockwise = 1;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 3;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.RTVFormats[1] = DXGI_FORMAT_R16G16_FLOAT;
	psoDesc.RTVFormats[2] = DXGI_FORMAT_R16G16_FLOAT;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;

	auto hresult = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipelineState.getAddressOf()));
	if (hresult != 0)
		return false;

	return true;
}

bool GBufferRenderer::createTextures(const vx::uint2 &resolution, d3d::Device* device)
{
	D3D12_HEAP_PROPERTIES props
	{
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		0,
		0
	};

	D3D12_RESOURCE_DESC descs[4];
	descs[0].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	descs[0].Alignment = 64 KBYTE;
	descs[0].Width = resolution.x;
	descs[0].Height = resolution.y;
	descs[0].DepthOrArraySize = 2;
	descs[0].MipLevels = 0;
	descs[0].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	descs[0].SampleDesc.Count = 1;
	descs[0].SampleDesc.Quality = 0;
	descs[0].Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	descs[0].Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	descs[1] = descs[0];
	descs[1].Format = DXGI_FORMAT_R16G16_FLOAT;

	descs[2] = descs[1];

	descs[3] = descs[0];
	descs[3].Format = DXGI_FORMAT_D32_FLOAT;
	descs[3].Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	auto szDiffuse = device->getDevice()->GetResourceAllocationInfo(1, 1, &descs[0]);
	auto szNormal = device->getDevice()->GetResourceAllocationInfo(1, 1, &descs[1]);
	auto szVelocity = device->getDevice()->GetResourceAllocationInfo(1, 1, &descs[2]);
	auto szDepth = device->getDevice()->GetResourceAllocationInfo(1, 1, &descs[3]);

	auto heapSize = szDiffuse.SizeInBytes + szNormal.SizeInBytes + szVelocity.SizeInBytes + szDepth.SizeInBytes;
	if (!m_gbufferHeap.createRtHeap(heapSize, D3D12_HEAP_TYPE_DEFAULT, device))
		return false;

	D3D12_CLEAR_VALUE diffuseOptimizedClearValue = {};
	diffuseOptimizedClearValue.Color[0] = 1.0f;
	diffuseOptimizedClearValue.Color[1] = 0.0f;
	diffuseOptimizedClearValue.Color[2] = 0.0f;
	diffuseOptimizedClearValue.Color[3] = 1.0f;
	diffuseOptimizedClearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	if (!m_gbufferHeap.createResource(descs[0], 0, D3D12_RESOURCE_STATE_RENDER_TARGET, &diffuseOptimizedClearValue, m_diffuseSlice.getAddressOf(), device))
		return false;

	D3D12_CLEAR_VALUE normalOptimizedClearValue = {};
	normalOptimizedClearValue.Color[0] = 1.0f;
	normalOptimizedClearValue.Color[1] = 0.0f;
	normalOptimizedClearValue.Color[2] = 0.0f;
	normalOptimizedClearValue.Color[3] = 1.0f;
	normalOptimizedClearValue.Format = DXGI_FORMAT_R16G16_FLOAT;

	if (!m_gbufferHeap.createResource(descs[1], szDiffuse.SizeInBytes, D3D12_RESOURCE_STATE_RENDER_TARGET, &normalOptimizedClearValue, m_normalSlice.getAddressOf(), device))
		return false;

	if (!m_gbufferHeap.createResource(descs[2], szDiffuse.SizeInBytes + szNormal.SizeInBytes, D3D12_RESOURCE_STATE_RENDER_TARGET, &normalOptimizedClearValue, m_velocitySlice.getAddressOf(), device))
		return false;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	if (!m_gbufferHeap.createResource(descs[3], szDiffuse.SizeInBytes + szNormal.SizeInBytes + szVelocity.SizeInBytes, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthOptimizedClearValue, m_depthSlice.getAddressOf(), device))
		return false;

	return true;
}

bool GBufferRenderer::createDescriptorHeap(d3d::Device* device)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.NodeMask = 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NumDescriptors = 3;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	if (!m_descriptorHeapRt.create(desc, device))
		return false;

	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	if (!m_descriptorHeapDs.create(desc, device))
		return false;

	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NumDescriptors = 4;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	return m_descriptorHeapSrv.create(desc, device);
}

bool GBufferRenderer::initialize(const vx::uint2 &resolution, d3d::Device* device, ShaderManager* shaderManager)
{
	if (!loadShaders(shaderManager))
		return false;

	if (!createRootSignature(device->getDevice()))
		return false;

	if (!createPipelineState(device->getDevice(), shaderManager))
		return false;

	if (!createTextures(resolution, device))
		return false;

	if (!createDescriptorHeap(device))
		return false;

	D3D12_DEPTH_STENCIL_VIEW_DESC depthViewDesc;
	depthViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
	depthViewDesc.Flags = D3D12_DSV_FLAG_NONE;
	depthViewDesc.Texture2DArray.ArraySize = 2;
	depthViewDesc.Texture2DArray.FirstArraySlice = 0;
	depthViewDesc.Texture2DArray.MipSlice = 0;
	device->getDevice()->CreateDepthStencilView(m_depthSlice.get(), &depthViewDesc, m_descriptorHeapDs.getHandleCpu());

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.ArraySize = 2;
	rtvDesc.Texture2DArray.FirstArraySlice = 0;
	rtvDesc.Texture2DArray.MipSlice = 0;
	rtvDesc.Texture2DArray.PlaneSlice = 0;

	auto handle = m_descriptorHeapRt.getHandleCpu();
	device->getDevice()->CreateRenderTargetView(m_diffuseSlice.get(), &rtvDesc, handle);

	handle.offset(1);
	rtvDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
	device->getDevice()->CreateRenderTargetView(m_normalSlice.get(), &rtvDesc, handle);

	handle.offset(1);
	rtvDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
	device->getDevice()->CreateRenderTargetView(m_velocitySlice.get(), &rtvDesc, handle);

	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2DArray.ArraySize = 2;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.MipLevels = 1;
		srvDesc.Texture2DArray.MostDetailedMip = 0 ;
		srvDesc.Texture2DArray.PlaneSlice = 0;
		srvDesc.Texture2DArray.ResourceMinLODClamp = 0;

		auto handle = m_descriptorHeapSrv.getHandleCpu();
		device->getDevice()->CreateShaderResourceView(m_diffuseSlice.get(), &srvDesc, handle);

		handle.offset(1);
		srvDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
		device->getDevice()->CreateShaderResourceView(m_normalSlice.get(), &srvDesc, handle);

		handle.offset(1);
		srvDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
		device->getDevice()->CreateShaderResourceView(m_velocitySlice.get(), &srvDesc, handle);
	}

	return true;
}

void GBufferRenderer::submitCommands(ID3D12GraphicsCommandList* cmdList)
{
	const f32 clearColor[] = { 1.0f, 0.0f, 0.0f, 1 };

	auto handleCpu = m_descriptorHeapRt.getHandleCpu();
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[3];
	rtvHandles[0] = handleCpu;
	handleCpu.offset(1);
	rtvHandles[1] = handleCpu;
	handleCpu.offset(1);
	rtvHandles[2] = handleCpu;

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandles[1];
	dsvHandles[0] = m_descriptorHeapDs.getHandleCpu();

	cmdList->OMSetRenderTargets(3, rtvHandles, FALSE, dsvHandles);
	cmdList->ClearRenderTargetView(rtvHandles[0], clearColor, 0, nullptr);
	cmdList->ClearRenderTargetView(rtvHandles[1], clearColor, 0, nullptr);
	cmdList->ClearRenderTargetView(rtvHandles[2], clearColor, 0, nullptr);
	cmdList->ClearDepthStencilView(dsvHandles[0], D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	cmdList->SetGraphicsRootSignature(m_rootSignature.get());
	cmdList->SetPipelineState(m_pipelineState.get());
}

void GBufferRenderer::bindSrvBegin(ID3D12GraphicsCommandList* cmdList)
{
	ID3D12DescriptorHeap* heaps[] =
	{
		m_descriptorHeapSrv.get()
	};

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_diffuseSlice.get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_normalSlice.get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_velocitySlice.get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	cmdList->SetDescriptorHeaps(1, heaps);

	cmdList->SetGraphicsRootDescriptorTable(0, m_descriptorHeapSrv->GetGPUDescriptorHandleForHeapStart());
}

void GBufferRenderer::bindSrvEnd(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_diffuseSlice.get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_normalSlice.get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_velocitySlice.get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
}