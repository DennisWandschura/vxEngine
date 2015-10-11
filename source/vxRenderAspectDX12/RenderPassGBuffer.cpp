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

#include "RenderPassGBuffer.h"
#include <d3d12.h>
#include "d3dx12.h"
#include "ShaderManager.h"
#include "Device.h"
#include "ResourceManager.h"
#include "ResourceView.h"
#include "DrawIndexedIndirectCommand.h"
#include "CommandAllocator.h"
#include "GpuProfiler.h"

struct RenderPassGBuffer::ColdData
{
	enum TextureIndex { Diffuse, Normal, Velocity, Depth, ZBuffer, Surface, TextureCount };

	D3D12_RESOURCE_DESC resDescs[TextureCount];
};

RenderPassGBuffer::RenderPassGBuffer(DrawIndexedIndirectCommand* drawCmd)
	:RenderPass(),
	m_drawCmd(drawCmd),
	m_buildList(0),
	m_coldData(new ColdData())
{
	createTextureDescriptions();
}

RenderPassGBuffer::~RenderPassGBuffer()
{

}

void RenderPassGBuffer::createTextureDescriptions()
{
	m_coldData->resDescs[ColdData::TextureIndex::Diffuse].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	m_coldData->resDescs[ColdData::TextureIndex::Diffuse].Alignment = 64 KBYTE;
	m_coldData->resDescs[ColdData::TextureIndex::Diffuse].Width = s_resolution.x;
	m_coldData->resDescs[ColdData::TextureIndex::Diffuse].Height = s_resolution.y;
	m_coldData->resDescs[ColdData::TextureIndex::Diffuse].DepthOrArraySize = 1;
	m_coldData->resDescs[ColdData::TextureIndex::Diffuse].MipLevels = 1;
	m_coldData->resDescs[ColdData::TextureIndex::Diffuse].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_coldData->resDescs[ColdData::TextureIndex::Diffuse].SampleDesc.Count = 1;
	m_coldData->resDescs[ColdData::TextureIndex::Diffuse].SampleDesc.Quality = 0;
	m_coldData->resDescs[ColdData::TextureIndex::Diffuse].Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	m_coldData->resDescs[ColdData::TextureIndex::Diffuse].Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	m_coldData->resDescs[ColdData::TextureIndex::Normal].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	m_coldData->resDescs[ColdData::TextureIndex::Normal].Alignment = 64 KBYTE;
	m_coldData->resDescs[ColdData::TextureIndex::Normal].Width = s_resolution.x;
	m_coldData->resDescs[ColdData::TextureIndex::Normal].Height = s_resolution.y;
	m_coldData->resDescs[ColdData::TextureIndex::Normal].DepthOrArraySize = 1;
	m_coldData->resDescs[ColdData::TextureIndex::Normal].MipLevels = 1;
	m_coldData->resDescs[ColdData::TextureIndex::Normal].Format = DXGI_FORMAT_R16G16_FLOAT;
	m_coldData->resDescs[ColdData::TextureIndex::Normal].SampleDesc.Count = 1;
	m_coldData->resDescs[ColdData::TextureIndex::Normal].SampleDesc.Quality = 0;
	m_coldData->resDescs[ColdData::TextureIndex::Normal].Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	m_coldData->resDescs[ColdData::TextureIndex::Normal].Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	m_coldData->resDescs[ColdData::TextureIndex::Velocity].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	m_coldData->resDescs[ColdData::TextureIndex::Velocity].Alignment = 64 KBYTE;
	m_coldData->resDescs[ColdData::TextureIndex::Velocity].Width = s_resolution.x;
	m_coldData->resDescs[ColdData::TextureIndex::Velocity].Height = s_resolution.y;
	m_coldData->resDescs[ColdData::TextureIndex::Velocity].DepthOrArraySize = 1;
	m_coldData->resDescs[ColdData::TextureIndex::Velocity].MipLevels = 1;
	m_coldData->resDescs[ColdData::TextureIndex::Velocity].Format = DXGI_FORMAT_R16G16_FLOAT;
	m_coldData->resDescs[ColdData::TextureIndex::Velocity].SampleDesc.Count = 1;
	m_coldData->resDescs[ColdData::TextureIndex::Velocity].SampleDesc.Quality = 0;
	m_coldData->resDescs[ColdData::TextureIndex::Velocity].Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	m_coldData->resDescs[ColdData::TextureIndex::Velocity].Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	m_coldData->resDescs[ColdData::TextureIndex::Depth].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	m_coldData->resDescs[ColdData::TextureIndex::Depth].Alignment = 64 KBYTE;
	m_coldData->resDescs[ColdData::TextureIndex::Depth].Width = s_resolution.x;
	m_coldData->resDescs[ColdData::TextureIndex::Depth].Height = s_resolution.y;
	m_coldData->resDescs[ColdData::TextureIndex::Depth].DepthOrArraySize = 1;
	m_coldData->resDescs[ColdData::TextureIndex::Depth].MipLevels = 1;
	m_coldData->resDescs[ColdData::TextureIndex::Depth].Format = DXGI_FORMAT_R32G8X24_TYPELESS;
	m_coldData->resDescs[ColdData::TextureIndex::Depth].SampleDesc.Count = 1;
	m_coldData->resDescs[ColdData::TextureIndex::Depth].SampleDesc.Quality = 0;
	m_coldData->resDescs[ColdData::TextureIndex::Depth].Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	m_coldData->resDescs[ColdData::TextureIndex::Depth].Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	m_coldData->resDescs[ColdData::TextureIndex::ZBuffer].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	m_coldData->resDescs[ColdData::TextureIndex::ZBuffer].Alignment = 64 KBYTE;
	m_coldData->resDescs[ColdData::TextureIndex::ZBuffer].Width = s_resolution.x;
	m_coldData->resDescs[ColdData::TextureIndex::ZBuffer].Height = s_resolution.y;
	m_coldData->resDescs[ColdData::TextureIndex::ZBuffer].DepthOrArraySize = 1;
	m_coldData->resDescs[ColdData::TextureIndex::ZBuffer].MipLevels = 6;
	m_coldData->resDescs[ColdData::TextureIndex::ZBuffer].Format = DXGI_FORMAT_R32_FLOAT;
	m_coldData->resDescs[ColdData::TextureIndex::ZBuffer].SampleDesc.Count = 1;
	m_coldData->resDescs[ColdData::TextureIndex::ZBuffer].SampleDesc.Quality = 0;
	m_coldData->resDescs[ColdData::TextureIndex::ZBuffer].Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	m_coldData->resDescs[ColdData::TextureIndex::ZBuffer].Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	m_coldData->resDescs[ColdData::TextureIndex::Surface].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	m_coldData->resDescs[ColdData::TextureIndex::Surface].Alignment = 64 KBYTE;
	m_coldData->resDescs[ColdData::TextureIndex::Surface].Width = s_resolution.x;
	m_coldData->resDescs[ColdData::TextureIndex::Surface].Height = s_resolution.y;
	m_coldData->resDescs[ColdData::TextureIndex::Surface].DepthOrArraySize = 1;
	m_coldData->resDescs[ColdData::TextureIndex::Surface].MipLevels = 1;
	m_coldData->resDescs[ColdData::TextureIndex::Surface].Format = DXGI_FORMAT_R16G16_FLOAT;
	m_coldData->resDescs[ColdData::TextureIndex::Surface].SampleDesc.Count = 1;
	m_coldData->resDescs[ColdData::TextureIndex::Surface].SampleDesc.Quality = 0;
	m_coldData->resDescs[ColdData::TextureIndex::Surface].Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	m_coldData->resDescs[ColdData::TextureIndex::Surface].Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
}

void RenderPassGBuffer::getRequiredMemory(u64* heapSizeBuffer, u32* bufferCount, u64* heapSizeTexture, u32* textureCount, u64* heapSizeRtDs, u32* rtDsCount, ID3D12Device* device)
{
	auto allocInfo = device->GetResourceAllocationInfo(1, ColdData::TextureCount, m_coldData->resDescs);

	*heapSizeRtDs += allocInfo.SizeInBytes;
	*rtDsCount += ColdData::TextureCount;
}

bool RenderPassGBuffer::loadShaders(d3d::ShaderManager* shaderManager)
{
	if (!shaderManager->loadShader(L"MeshVertex.cso"))
		return false;

	//if (!shaderManager->loadShader("DeepGBufferGs.cso", L"../../lib/DeepGBufferGs.cso", d3d::ShaderType::Geometry))
	//	return false;

	if (!shaderManager->loadShader(L"PsGBuffer.cso"))
		return false;

	return true;
}

bool RenderPassGBuffer::createData(ID3D12Device* device)
{
	return createTextures(s_resourceManager, s_resolution, device);
}

bool RenderPassGBuffer::createRootSignature(ID3D12Device* device)
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
	sampler.Filter = D3D12_FILTER_ANISOTROPIC;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 4;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = 0.0f;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(2, rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	return m_rootSignature.create(&rootSignatureDesc, device);
}

bool RenderPassGBuffer::createPipelineState(ID3D12Device* device, d3d::ShaderManager* shaderManager)
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

	DXGI_FORMAT rtvFormats[] =
	{
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_R16G16_FLOAT,
		DXGI_FORMAT_R16G16_FLOAT,
		DXGI_FORMAT_R16G16_FLOAT,
	};

	d3d::PipelineStateDescInput inputDesc;
	inputDesc.rootSignature = m_rootSignature.get();
	inputDesc.inputLayout = { inputLayout, _countof(inputLayout) };
	inputDesc.shaderDesc.vs = s_shaderManager->getShader(L"MeshVertex.cso");
	//inputDesc.shaderDesc.gs = s_shaderManager->getShader("DeepGBufferGs.cso");
	inputDesc.shaderDesc.ps = s_shaderManager->getShader(L"PsGBuffer.cso");
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	inputDesc.rtvFormats = rtvFormats;
	inputDesc.rtvCount = 4;
	inputDesc.dsvFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);

	return d3d::PipelineState::create(desc, &m_pipelineState, device);
}

bool RenderPassGBuffer::createTextures(d3d::ResourceManager* resourceManager, const vx::uint2 &resolution, ID3D12Device* device)
{
	D3D12_CLEAR_VALUE clearValues[ColdData::TextureCount];
	// diffuse
	clearValues[ColdData::Diffuse].Color[0] = 0.0f;
	clearValues[ColdData::Diffuse].Color[1] = 0.0f;
	clearValues[ColdData::Diffuse].Color[2] = 0.0f;
	clearValues[ColdData::Diffuse].Color[3] = 0.0f;
	clearValues[ColdData::Diffuse].Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// normal
	clearValues[ColdData::Normal].Color[0] = 0.0f;
	clearValues[ColdData::Normal].Color[1] = 0.0f;
	clearValues[ColdData::Normal].Color[2] = 0.0f;
	clearValues[ColdData::Normal].Color[3] = 0.0f;
	clearValues[ColdData::Normal].Format = DXGI_FORMAT_R16G16_FLOAT;

	clearValues[ColdData::Velocity].Color[0] = 0.0f;
	clearValues[ColdData::Velocity].Color[1] = 0.0f;
	clearValues[ColdData::Velocity].Color[2] = 0.0f;
	clearValues[ColdData::Velocity].Color[3] = 0.0f;
	clearValues[ColdData::Velocity].Format = DXGI_FORMAT_R16G16_FLOAT;

	// detph
	clearValues[ColdData::Depth].Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	clearValues[ColdData::Depth].DepthStencil.Depth = 1.0f;
	clearValues[ColdData::Depth].DepthStencil.Stencil = 0;

	// zbuffer
	clearValues[ColdData::ZBuffer].Color[0] = 1.0f;
	clearValues[ColdData::ZBuffer].Color[1] = 0.0f;
	clearValues[ColdData::ZBuffer].Color[2] = 0.0f;
	clearValues[ColdData::ZBuffer].Color[3] = 0.0f;
	clearValues[ColdData::ZBuffer].Format = DXGI_FORMAT_R32_FLOAT;

	// surface
	clearValues[ColdData::Surface].Color[0] = 0.0f;
	clearValues[ColdData::Surface].Color[1] = 0.0f;
	clearValues[ColdData::Surface].Color[2] = 0.0f;
	clearValues[ColdData::Surface].Color[3] = 0.0f;
	clearValues[ColdData::Surface].Format = DXGI_FORMAT_R16G16_FLOAT;

	const wchar_t* names[ColdData::TextureCount];
	names[ColdData::Diffuse] = L"gbufferAlbedo";
	names[ColdData::Normal] = L"gbufferNormal";
	names[ColdData::Velocity] = L"gbufferVelocity";
	names[ColdData::Depth] = L"gbufferDepth";
	names[ColdData::ZBuffer] = L"zBuffer";
	names[ColdData::Surface] = L"gbufferSurface";

	D3D12_RESOURCE_STATES states[ColdData::TextureCount];
	states[ColdData::Diffuse] = D3D12_RESOURCE_STATE_RENDER_TARGET;
	states[ColdData::Normal] = D3D12_RESOURCE_STATE_RENDER_TARGET;
	states[ColdData::Velocity] = D3D12_RESOURCE_STATE_RENDER_TARGET;
	states[ColdData::Depth] = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	states[ColdData::ZBuffer] = D3D12_RESOURCE_STATE_GENERIC_READ;
	states[ColdData::Surface] = D3D12_RESOURCE_STATE_RENDER_TARGET;

	CreateResourceDesc desc;
	for (u32 i = 0; i < ColdData::TextureCount; ++i)
	{
		auto allocInfo = device->GetResourceAllocationInfo(1, 1, &m_coldData->resDescs[i]);

		desc.state = states[i];
		desc.clearValue = &clearValues[i];
		desc.size = allocInfo.SizeInBytes;
		desc.resDesc = &m_coldData->resDescs[i];

		if (!resourceManager->createTextureRtDs(names[i], desc))
			return false;
	}

	return true;
}

bool RenderPassGBuffer::createDescriptorHeap(ID3D12Device* device)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.NodeMask = 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NumDescriptors = 4;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	if (!m_descriptorHeapRt.create(desc, device))
		return false;

	desc.NumDescriptors = 2;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	if (!m_descriptorHeapDs.create(desc, device))
		return false;

	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NumDescriptors = 6;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (!m_descriptorHeapBuffers.create(desc, device))
		return false;

	return true;
}

void RenderPassGBuffer::createBufferViews(d3d::ResourceManager* resourceManager, ID3D12Device* device)
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
	device->CreateShaderResourceView(transformBuffer->get(), transformBufferViewDesc, handle);

	handle.offset(1);
	device->CreateShaderResourceView(materialBuffer->get(), materialBufferViewDesc, handle);

	handle.offset(1);
	device->CreateShaderResourceView(transformBufferPrev->get(), transformBufferPrevViewDesc, handle);

	handle.offset(1);
	device->CreateShaderResourceView(srgbTexture->get(), srgbTextureViewDesc, handle);

	handle.offset(1);
	device->CreateShaderResourceView(rgbTexture->get(), rgbTextureViewDesc, handle);
}

bool RenderPassGBuffer::initialize(ID3D12Device* device, d3d::CommandAllocator* allocators, u32 frameCount)
{

	if (!loadShaders(s_shaderManager))
		return false;

	if (!createRootSignature(device))
		return false;

	if (!createPipelineState(device, s_shaderManager))
		return false;

	if (!createDescriptorHeap(device))
		return false;

	if (!createCommandLists(device, D3D12_COMMAND_LIST_TYPE_DIRECT, allocators, frameCount))
		return false;

	auto gbufferAlbedo = s_resourceManager->getTextureRtDs(L"gbufferAlbedo");
	auto gbufferNormal = s_resourceManager->getTextureRtDs(L"gbufferNormal");
	auto gbufferVelocity = s_resourceManager->getTextureRtDs(L"gbufferVelocity");
	auto gbufferSurface = s_resourceManager->getTextureRtDs(L"gbufferSurface");
	auto gbufferDepth = s_resourceManager->getTextureRtDs(L"gbufferDepth");

	D3D12_DEPTH_STENCIL_VIEW_DESC depthViewDesc;
	depthViewDesc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	depthViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthViewDesc.Flags = D3D12_DSV_FLAG_NONE;
	depthViewDesc.Texture2D.MipSlice = 0;
	auto dsHandle = m_descriptorHeapDs.getHandleCpu();
	device->CreateDepthStencilView(gbufferDepth->get(), &depthViewDesc, dsHandle);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	auto rtvHandle = m_descriptorHeapRt.getHandleCpu();
	device->CreateRenderTargetView(gbufferAlbedo->get(), &rtvDesc, rtvHandle);

	rtvHandle.offset(1);
	rtvDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
	device->CreateRenderTargetView(gbufferNormal->get(), &rtvDesc, rtvHandle);

	rtvHandle.offset(1);
	rtvDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
	device->CreateRenderTargetView(gbufferVelocity->get(), &rtvDesc, rtvHandle);

	rtvHandle.offset(1);
	rtvDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
	device->CreateRenderTargetView(gbufferSurface->get(), &rtvDesc, rtvHandle);

	createBufferViews(s_resourceManager, device);

	return true;
}

void RenderPassGBuffer::shutdown()
{
	m_descriptorHeapRt.destroy();
	m_descriptorHeapDs.destroy();
	m_descriptorHeapBuffers.destroy();

	m_pipelineState.destroy();
	m_rootSignature.destroy();
	m_drawCmd = nullptr;
}

void RenderPassGBuffer::buildCommands(d3d::CommandAllocator* currentAllocator, u32 frameIndex)
{
	auto drawCount = m_drawCmd->getCount();
	if (drawCount != 0)
	{
		auto &commandList = m_commandLists[frameIndex];
		m_currentCommandList = &commandList;

		auto gbufferDepth = s_resourceManager->getTextureRtDs(L"gbufferDepth");

		auto hresult = commandList->Reset(currentAllocator->get(), m_pipelineState.get());
		VX_ASSERT(hresult == 0);
		// copylayer 0 depth buffer to layer 1
		/*D3D12_RESOURCE_BARRIER barriers[2];
		barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(gbufferDepth->get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_SOURCE, 0);
		barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(gbufferDepth->get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_DEST, 1);
		m_commandList->ResourceBarrier(2, barriers);

		CD3DX12_TEXTURE_COPY_LOCATION src(gbufferDepth->get(), 0);
		CD3DX12_TEXTURE_COPY_LOCATION dst(gbufferDepth->get(), 1);
		m_commandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

		barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(gbufferDepth->get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE, 0);
		barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(gbufferDepth->get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_DEPTH_WRITE, 1);
		m_commandList->ResourceBarrier(2, barriers);*/

		//	const f32 clearColor[] = { 1.0f, 0.0f, 0.0f, 1 };
		const f32 clearColor0[] = { 0.0f, 0.0f, 0.0f, 0 };

		auto handleCpu = m_descriptorHeapRt.getHandleCpu();
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[4];
		rtvHandles[0] = handleCpu;
		handleCpu.offset(1);
		rtvHandles[1] = handleCpu;
		handleCpu.offset(1);
		rtvHandles[2] = handleCpu;
		handleCpu.offset(1);
		rtvHandles[3] = handleCpu;

		auto dsvHandle = m_descriptorHeapDs.getHandleCpu();;
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandles[1];
		dsvHandles[0] = dsvHandle;

		//dsvHandle.offset(1);
		//auto dsvHandleClear = dsvHandle;

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

		s_gpuProfiler->queryBegin("gbuffer", &commandList);

		commandList->RSSetViewports(1, &viewPort);
		commandList->RSSetScissorRects(1, &rectScissor);

		commandList->OMSetRenderTargets(4, rtvHandles, FALSE, dsvHandles);
		commandList->ClearRenderTargetView(rtvHandles[0], clearColor0, 0, nullptr);
		commandList->ClearRenderTargetView(rtvHandles[1], clearColor0, 0, nullptr);
		commandList->ClearRenderTargetView(rtvHandles[2], clearColor0, 0, nullptr);
		commandList->ClearRenderTargetView(rtvHandles[3], clearColor0, 0, nullptr);

		commandList->ClearDepthStencilView(dsvHandles[0], D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		commandList->SetDescriptorHeaps(1, heaps);
		commandList->SetGraphicsRootSignature(m_rootSignature.get());
		commandList->SetGraphicsRootDescriptorTable(0, m_descriptorHeapBuffers->GetGPUDescriptorHandleForHeapStart());
		commandList->SetGraphicsRootDescriptorTable(1, m_descriptorHeapBuffers->GetGPUDescriptorHandleForHeapStart());

		D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[2];
		vertexBufferViews[0] = s_resourceManager->getResourceView("meshVertexBufferView")->vbv;
		vertexBufferViews[1] = s_resourceManager->getResourceView("meshDrawIdBufferView")->vbv;
		auto indexBufferView = s_resourceManager->getResourceView("meshIndexBufferView")->ibv;

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 2, vertexBufferViews);
		commandList->IASetIndexBuffer(&indexBufferView);

		m_drawCmd->draw(commandList.get());

		s_gpuProfiler->queryEnd(&commandList);

		hresult = commandList->Close();
		m_buildList = 1;
		VX_ASSERT(hresult == 0);
	}
}

void RenderPassGBuffer::submitCommands(Graphics::CommandQueue* queue)
{
	if (m_buildList != 0)
	{
		queue->pushCommandList(m_currentCommandList);
		m_buildList = 0;
	}
}