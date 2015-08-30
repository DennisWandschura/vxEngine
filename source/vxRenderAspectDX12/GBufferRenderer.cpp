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
#include "ResourceManager.h"
#include "ResourceView.h"

struct GBufferRenderer::ColdData
{
	enum TextureIndex {Diffuse, NormalVelocity, Depth, ZBuffer0, ZBuffer1, TextureCount};

	D3D12_RESOURCE_DESC resDescs[TextureCount];
};

GBufferRenderer::GBufferRenderer(ID3D12CommandAllocator* cmdAlloc, u32 countOffset)
	:m_commandList(),
	m_cmdAlloc(cmdAlloc),
	m_depthSlice(nullptr),
	m_countOffset(countOffset),
	m_drawCount(0),
	m_coldData(new ColdData())
{
	createTextureDescriptions();
}

GBufferRenderer::~GBufferRenderer()
{

}

void GBufferRenderer::createTextureDescriptions()
{
	m_coldData->resDescs[ColdData::TextureIndex::Diffuse].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	m_coldData->resDescs[ColdData::TextureIndex::Diffuse].Alignment = 64 KBYTE;
	m_coldData->resDescs[ColdData::TextureIndex::Diffuse].Width = s_resolution.x;
	m_coldData->resDescs[ColdData::TextureIndex::Diffuse].Height = s_resolution.y;
	m_coldData->resDescs[ColdData::TextureIndex::Diffuse].DepthOrArraySize = 2;
	m_coldData->resDescs[ColdData::TextureIndex::Diffuse].MipLevels = 1;
	m_coldData->resDescs[ColdData::TextureIndex::Diffuse].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_coldData->resDescs[ColdData::TextureIndex::Diffuse].SampleDesc.Count = 1;
	m_coldData->resDescs[ColdData::TextureIndex::Diffuse].SampleDesc.Quality = 0;
	m_coldData->resDescs[ColdData::TextureIndex::Diffuse].Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	m_coldData->resDescs[ColdData::TextureIndex::Diffuse].Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	m_coldData->resDescs[ColdData::TextureIndex::NormalVelocity].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	m_coldData->resDescs[ColdData::TextureIndex::NormalVelocity].Alignment = 64 KBYTE;
	m_coldData->resDescs[ColdData::TextureIndex::NormalVelocity].Width = s_resolution.x;
	m_coldData->resDescs[ColdData::TextureIndex::NormalVelocity].Height = s_resolution.y;
	m_coldData->resDescs[ColdData::TextureIndex::NormalVelocity].DepthOrArraySize = 2;
	m_coldData->resDescs[ColdData::TextureIndex::NormalVelocity].MipLevels = 1;
	m_coldData->resDescs[ColdData::TextureIndex::NormalVelocity].Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	m_coldData->resDescs[ColdData::TextureIndex::NormalVelocity].SampleDesc.Count = 1;
	m_coldData->resDescs[ColdData::TextureIndex::NormalVelocity].SampleDesc.Quality = 0;
	m_coldData->resDescs[ColdData::TextureIndex::NormalVelocity].Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	m_coldData->resDescs[ColdData::TextureIndex::NormalVelocity].Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	m_coldData->resDescs[ColdData::TextureIndex::Depth].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	m_coldData->resDescs[ColdData::TextureIndex::Depth].Alignment = 64 KBYTE;
	m_coldData->resDescs[ColdData::TextureIndex::Depth].Width = s_resolution.x;
	m_coldData->resDescs[ColdData::TextureIndex::Depth].Height = s_resolution.y;
	m_coldData->resDescs[ColdData::TextureIndex::Depth].DepthOrArraySize = 2;
	m_coldData->resDescs[ColdData::TextureIndex::Depth].MipLevels = 1;
	m_coldData->resDescs[ColdData::TextureIndex::Depth].Format = DXGI_FORMAT_R32_TYPELESS;
	m_coldData->resDescs[ColdData::TextureIndex::Depth].SampleDesc.Count = 1;
	m_coldData->resDescs[ColdData::TextureIndex::Depth].SampleDesc.Quality = 0;
	m_coldData->resDescs[ColdData::TextureIndex::Depth].Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	m_coldData->resDescs[ColdData::TextureIndex::Depth].Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	m_coldData->resDescs[ColdData::TextureIndex::ZBuffer0].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	m_coldData->resDescs[ColdData::TextureIndex::ZBuffer0].Alignment = 64 KBYTE;
	m_coldData->resDescs[ColdData::TextureIndex::ZBuffer0].Width = s_resolution.x;
	m_coldData->resDescs[ColdData::TextureIndex::ZBuffer0].Height = s_resolution.y;
	m_coldData->resDescs[ColdData::TextureIndex::ZBuffer0].DepthOrArraySize = 1;
	m_coldData->resDescs[ColdData::TextureIndex::ZBuffer0].MipLevels = 6;
	m_coldData->resDescs[ColdData::TextureIndex::ZBuffer0].Format = DXGI_FORMAT_R32_FLOAT;
	m_coldData->resDescs[ColdData::TextureIndex::ZBuffer0].SampleDesc.Count = 1;
	m_coldData->resDescs[ColdData::TextureIndex::ZBuffer0].SampleDesc.Quality = 0;
	m_coldData->resDescs[ColdData::TextureIndex::ZBuffer0].Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	m_coldData->resDescs[ColdData::TextureIndex::ZBuffer0].Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	m_coldData->resDescs[ColdData::TextureIndex::ZBuffer1] = m_coldData->resDescs[ColdData::TextureIndex::ZBuffer0];
}

void GBufferRenderer::getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device)
{
	auto allocInfo = device->GetResourceAllocationInfo(1, 5, m_coldData->resDescs);

	*heapSizeRtDs += allocInfo.SizeInBytes;
}

bool GBufferRenderer::loadShaders(d3d::ShaderManager* shaderManager)
{
	if (!shaderManager->loadShader("MeshVertex.cso", L"../../lib/MeshVertex.cso", d3d::ShaderType::Vertex))
		return false;

	if (!shaderManager->loadShader("DeepGBufferGs.cso", L"../../lib/DeepGBufferGs.cso", d3d::ShaderType::Geometry))
		return false;

	if (!shaderManager->loadShader("PsGBuffer.cso", L"../../lib/PsGBuffer.cso", d3d::ShaderType::Pixel))
		return false;

	return true;
}

bool GBufferRenderer::createRootSignature(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangesVS[2];
	rangesVS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 0);
	rangesVS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 0, 1);

	CD3DX12_DESCRIPTOR_RANGE rangePS[2];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 3, 0, 4);
	rangePS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 0);

	CD3DX12_ROOT_PARAMETER rootParameters[2];
	rootParameters[0].InitAsDescriptorTable(2, rangesVS, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsDescriptorTable(2, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);

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
	rootSignatureDesc.Init(2, rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

bool GBufferRenderer::createPipelineState(ID3D12Device* device, d3d::ShaderManager* shaderManager)
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
	psoDesc.NumRenderTargets = 2;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.RTVFormats[1] = DXGI_FORMAT_R16G16B16A16_FLOAT;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;

	auto hresult = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipelineState.getAddressOf()));
	if (hresult != 0)
		return false;

	return true;
}

bool GBufferRenderer::createTextures(d3d::ResourceManager* resourceManager, const vx::uint2 &resolution, ID3D12Device* device)
{
	D3D12_CLEAR_VALUE clearValues[ColdData::TextureCount];
	// diffuse
	clearValues[0].Color[0] = 1.0f;
	clearValues[0].Color[1] = 0.0f;
	clearValues[0].Color[2] = 0.0f;
	clearValues[0].Color[3] = 1.0f;
	clearValues[0].Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// normal
	clearValues[1].Color[0] = 1.0f;
	clearValues[1].Color[1] = 0.0f;
	clearValues[1].Color[2] = 0.0f;
	clearValues[1].Color[3] = 1.0f;
	clearValues[1].Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

	// detph
	clearValues[2].Format = DXGI_FORMAT_D32_FLOAT;
	clearValues[2].DepthStencil.Depth = 1.0f;
	clearValues[2].DepthStencil.Stencil = 0;

	// zbuffer
	clearValues[3].Color[0] = 1.0f;
	clearValues[3].Color[1] = 0.0f;
	clearValues[3].Color[2] = 0.0f;
	clearValues[3].Color[3] = 0.0f;
	clearValues[3].Format = DXGI_FORMAT_R32_FLOAT;

	clearValues[4] = clearValues[3];

	const wchar_t* names[ColdData::TextureCount];
	names[ColdData::Diffuse] = L"gbufferAlbedo";
	names[ColdData::NormalVelocity] = L"gbufferNormalVelocity";
	names[ColdData::Depth] = L"gbufferDepth";
	names[ColdData::ZBuffer0] = L"zBuffer0";
	names[ColdData::ZBuffer1] = L"zBuffer1";

	D3D12_RESOURCE_STATES states[ColdData::TextureCount];
	states[ColdData::Diffuse] = D3D12_RESOURCE_STATE_RENDER_TARGET;
	states[ColdData::NormalVelocity] = D3D12_RESOURCE_STATE_RENDER_TARGET;
	states[ColdData::Depth] = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	states[ColdData::ZBuffer0] = D3D12_RESOURCE_STATE_RENDER_TARGET;
	states[ColdData::ZBuffer1] = D3D12_RESOURCE_STATE_RENDER_TARGET;

	CreateResourceDesc desc;
	for (u32 i = 0; i < 5; ++i)
	{
		auto allocInfo = device->GetResourceAllocationInfo(1, 1, &m_coldData->resDescs[i]);

		desc.state = states[i];
		desc.clearValue = &clearValues[i];
		desc.size = allocInfo.SizeInBytes;
		desc.resDesc = &m_coldData->resDescs[i];

		if (!resourceManager->createTextureRtDs(names[i], desc))
			return false;
	}

	m_depthSlice = resourceManager->getTextureRtDs(names[ColdData::Depth]);
	m_diffuseSlice = resourceManager->getTextureRtDs(names[ColdData::Diffuse]);
	m_normalVelocitySlice = resourceManager->getTextureRtDs(names[ColdData::NormalVelocity]);

	return true;
}

bool GBufferRenderer::createDescriptorHeap(ID3D12Device* device)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.NodeMask = 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NumDescriptors = 3;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	if (!m_descriptorHeapRt.create(desc, device))
		return false;

	desc.NumDescriptors = 2;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	if (!m_descriptorHeapDs.create(desc, device))
		return false;

	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NumDescriptors = 4;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (!m_descriptorHeapSrv.create(desc, device))
		return false;

	desc.NumDescriptors = 6;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (!m_descriptorHeapBuffers.create(desc, device))
		return false;

	return true;
}

void GBufferRenderer::createBufferViews(d3d::ResourceManager* resourceManager, ID3D12Device* device)
{
	auto cameraBufferViewDesc = resourceManager->getConstantBufferView("cameraBufferView");
	auto transformBufferViewDesc = resourceManager->getShaderResourceView("transformBufferView");
	auto transformBufferPrevViewDesc = resourceManager->getShaderResourceView("transformBufferPrevView");
	auto materialBufferViewDesc = resourceManager->getShaderResourceView("materialBufferView");

	auto srgbTextureViewDesc = resourceManager->getShaderResourceView("srgbTextureView");
	auto rgbTextureViewDesc = resourceManager->getShaderResourceView("rgbTextureView");

	auto transformBuffer = resourceManager->getBuffer(L"transformBuffer");
	auto transformBufferPrev = resourceManager->getBuffer(L"transformBufferPrev");
	auto materialBuffer = resourceManager->getBuffer(L"materialBuffer");
	auto srgbTexture = resourceManager->getTexture(L"srgbTexture");
	auto rgbTexture = resourceManager->getTexture(L"rgbTexture");

	/*
	VS
	cbuffer CameraBuffer : register(b0)
	StructuredBuffer<TransformGpu> s_transforms : register(t0);
	StructuredBuffer<uint> s_materials : register(t1);
	StructuredBuffer<TransformGpu> s_transformsPrev : register(t2);

	PS
	cbuffer CameraBuffer : register(b0)
	Texture2DArray g_textureSrgba : register(t3);
	Texture2DArray g_textureRgba : register(t4);
	*/

	auto handle = m_descriptorHeapBuffers.getHandleCpu();
	device->CreateConstantBufferView(cameraBufferViewDesc, handle);

	handle.offset(1);
	device->CreateShaderResourceView(transformBuffer, transformBufferViewDesc, handle);

	handle.offset(1);
	device->CreateShaderResourceView(materialBuffer, materialBufferViewDesc, handle);

	handle.offset(1);
	device->CreateShaderResourceView(transformBufferPrev, transformBufferPrevViewDesc, handle);

	handle.offset(1);
	device->CreateShaderResourceView(srgbTexture, srgbTextureViewDesc, handle);

	handle.offset(1);
	device->CreateShaderResourceView(rgbTexture, rgbTextureViewDesc, handle);
}

bool GBufferRenderer::initialize(ID3D12Device* device, void* p)
{

	if (!loadShaders(s_shaderManager))
		return false;

	if (!createRootSignature(device))
		return false;

	if (!createPipelineState(device, s_shaderManager))
		return false;

	if (!createTextures(s_resourceManager, s_resolution, device))
		return false;

	if (!createDescriptorHeap(device))
		return false;

	if (device->CreateCommandList(1, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAlloc, m_pipelineState.get(), IID_PPV_ARGS(m_commandList.getAddressOf())) != 0)
		return false;

	m_commandList->Close();

	D3D12_DEPTH_STENCIL_VIEW_DESC depthViewDesc;
	depthViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
	depthViewDesc.Flags = D3D12_DSV_FLAG_NONE;
	depthViewDesc.Texture2DArray.ArraySize = 2;
	depthViewDesc.Texture2DArray.FirstArraySlice = 0;
	depthViewDesc.Texture2DArray.MipSlice = 0;
	auto dsHandle = m_descriptorHeapDs.getHandleCpu();
	device->CreateDepthStencilView(m_depthSlice, &depthViewDesc, dsHandle);

	dsHandle.offset(1);
	depthViewDesc.Texture2DArray.ArraySize = 1;
	device->CreateDepthStencilView(m_depthSlice, &depthViewDesc, dsHandle);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.ArraySize = 2;
	rtvDesc.Texture2DArray.FirstArraySlice = 0;
	rtvDesc.Texture2DArray.MipSlice = 0;
	rtvDesc.Texture2DArray.PlaneSlice = 0;

	auto handle = m_descriptorHeapRt.getHandleCpu();
	device->CreateRenderTargetView(m_diffuseSlice, &rtvDesc, handle);

	handle.offset(1);
	rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	device->CreateRenderTargetView(m_normalVelocitySlice, &rtvDesc, handle);

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
		device->CreateShaderResourceView(m_diffuseSlice, &srvDesc, handle);

		handle.offset(1);
		srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		device->CreateShaderResourceView(m_normalVelocitySlice, &srvDesc, handle);
	}

	createBufferViews(s_resourceManager, device);

	D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[1] = {};
	argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

	D3D12_COMMAND_SIGNATURE_DESC cmdSigDesc;
	cmdSigDesc.ByteStride = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
	cmdSigDesc.NodeMask = 0;
	cmdSigDesc.NumArgumentDescs = 1;
	cmdSigDesc.pArgumentDescs = argumentDescs;
	auto hresult = device->CreateCommandSignature(&cmdSigDesc, nullptr, IID_PPV_ARGS(m_commandSignature.getAddressOf()));
	if (hresult != 0)
		return false;

	return true;
}

void GBufferRenderer::shutdown()
{
	m_cmdAlloc = nullptr;
	m_commandList.destroy();

	m_diffuseSlice = nullptr;
	m_descriptorHeapRt.destroy();
	m_descriptorHeapDs.destroy();
	m_descriptorHeapSrv.destroy();
	m_descriptorHeapBuffers.destroy();

	m_pipelineState.destroy();
	m_rootSignature.destroy();
	m_commandSignature.destroy();
}

void GBufferRenderer::submitCommands(ID3D12CommandList** list, u32* index)
{
	auto hresult = m_commandList->Reset(m_cmdAlloc, m_pipelineState.get());
	VX_ASSERT(hresult == 0);
	// copylayer 0 depth buffer to layer 1
	CD3DX12_RESOURCE_BARRIER barriers[2];
	barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(m_depthSlice, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_SOURCE, 0);
	barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_depthSlice, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_DEST, 1);
	m_commandList->ResourceBarrier(2, barriers);

	CD3DX12_TEXTURE_COPY_LOCATION src(m_depthSlice, 0);
	CD3DX12_TEXTURE_COPY_LOCATION dst(m_depthSlice, 1);
	m_commandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(m_depthSlice, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE, 0);
	barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_depthSlice, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_DEPTH_WRITE, 1);
	m_commandList->ResourceBarrier(2, barriers);

	const f32 clearColor[] = { 1.0f, 0.0f, 0.0f, 1 };

	auto handleCpu = m_descriptorHeapRt.getHandleCpu();
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	rtvHandles[0] = handleCpu;
	handleCpu.offset(1);
	rtvHandles[1] = handleCpu;

	auto dsvHandle = m_descriptorHeapDs.getHandleCpu();;
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandles[1];
	dsvHandles[0] = dsvHandle;

	dsvHandle.offset(1);
	auto dsvHandleClear = dsvHandle;

	ID3D12DescriptorHeap* heaps[]
	{
		m_descriptorHeapBuffers.get()
	};

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

	m_commandList->RSSetViewports(1, &viewPort);
	m_commandList->RSSetScissorRects(1, &rectScissor);

	m_commandList->SetGraphicsRootSignature(m_rootSignature.get());

	m_commandList->SetDescriptorHeaps(1, heaps);

	m_commandList->OMSetRenderTargets(2, rtvHandles, FALSE, dsvHandles);
	m_commandList->ClearRenderTargetView(rtvHandles[0], clearColor, 0, nullptr);
	m_commandList->ClearRenderTargetView(rtvHandles[1], clearColor, 0, nullptr);

	m_commandList->ClearDepthStencilView(dsvHandleClear, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	m_commandList->SetGraphicsRootDescriptorTable(0, m_descriptorHeapBuffers->GetGPUDescriptorHandleForHeapStart());
	m_commandList->SetGraphicsRootDescriptorTable(1, m_descriptorHeapBuffers->GetGPUDescriptorHandleForHeapStart());

	if (m_drawCount != 0)
	{
		auto drawCmdBuffer = s_resourceManager->getBuffer(L"drawCmdBuffer");
		D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[2];
		vertexBufferViews[0] = s_resourceManager->getResourceView("meshVertexBufferView")->vbv;
		vertexBufferViews[1] = s_resourceManager->getResourceView("meshDrawIdBufferView")->vbv;
		auto indexBufferView = s_resourceManager->getResourceView("meshIndexBufferView")->ibv;

		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_commandList->IASetVertexBuffers(0, 2, vertexBufferViews);
		m_commandList->IASetIndexBuffer(&indexBufferView);

		//const auto countOffset = d3d::AlignedSizeType<D3D12_DRAW_INDEXED_ARGUMENTS, g_maxMeshInstances, 256>::size;
		m_commandList->ExecuteIndirect(m_commandSignature.get(), m_drawCount, drawCmdBuffer, 0, drawCmdBuffer, m_countOffset);
	}

	// draw meshes
	/*
	if (!m_drawCommands.empty())
	{
	

	D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[2];
	vertexBufferViews[0] = m_resourceManager.getResourceView("meshVertexBufferView")->vbv;
	vertexBufferViews[1] = m_resourceManager.getResourceView("meshDrawIdBufferView")->vbv;
	auto indexBufferView = m_resourceManager.getResourceView("meshIndexBufferView")->ibv;

	m_commandListDrawMesh->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandListDrawMesh->IASetVertexBuffers(0, 2, vertexBufferViews);
	m_commandListDrawMesh->IASetIndexBuffer(&indexBufferView);

	auto cmdCount = m_drawCommands.size();
	const auto countOffset = d3d::AlignedSizeType<D3D12_DRAW_INDEXED_ARGUMENTS, g_maxMeshInstances, 256>::size;
	m_commandListDrawMesh->ExecuteIndirect(m_commandSignature.get(), cmdCount, drawCmdBuffer, 0, drawCmdBuffer, countOffset);
	}
	*/

	hresult = m_commandList->Close();
	VX_ASSERT(hresult == 0);

	list[*index] = m_commandList.get();
	++(*index);
}