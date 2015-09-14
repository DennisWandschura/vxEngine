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
#include "RenderPassFilterRSM.h"
#include "ResourceManager.h"
#include "ShaderManager.h"
#include "CommandAllocator.h"

RenderPassFilterRSM::RenderPassFilterRSM(d3d::CommandAllocator* allocator)
	:RenderPass(),
	m_allocator(allocator),
	m_lightCount(0)
{

}

RenderPassFilterRSM::~RenderPassFilterRSM()
{

}

void RenderPassFilterRSM::getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device)
{
	const u32 textureCount = 10;
	const u32 resolution = 256;

	D3D12_RESOURCE_DESC resDesc;
	resDesc.Alignment = 64 KBYTE;
	resDesc.DepthOrArraySize = textureCount * 6;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Height = resolution;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Width = resolution;

	auto allocColor = device->GetResourceAllocationInfo(1, 1, &resDesc);

	resDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
	auto allocNormal = device->GetResourceAllocationInfo(1, 1, &resDesc);

	*heapSizeRtDs += allocColor.SizeInBytes + allocNormal.SizeInBytes;
}

bool RenderPassFilterRSM::createData(ID3D12Device* device)
{
	const u32 textureCount = 10;
	const u32 resolution = 256;

	D3D12_RESOURCE_DESC resDesc;
	resDesc.Alignment = 64 KBYTE;
	resDesc.DepthOrArraySize = textureCount * 6;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Height = resolution;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Width = resolution;

	auto alloc = device->GetResourceAllocationInfo(1, 1, &resDesc);

	D3D12_CLEAR_VALUE clearValues[1];
	clearValues[0].Color[0] = 0.0f;
	clearValues[0].Color[1] = 0.0f;
	clearValues[0].Color[2] = 0.0f;
	clearValues[0].Color[3] = 0.0f;
	clearValues[0].Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	CreateResourceDesc desc;
	desc.clearValue = clearValues;
	desc.resDesc = &resDesc;
	desc.size = alloc.SizeInBytes;
	desc.state = D3D12_RESOURCE_STATE_RENDER_TARGET;
	auto ptr = s_resourceManager->createTextureRtDs(L"rsmFilteredColor", desc);
	if (ptr == nullptr)
		return false;

	clearValues[0].Format = DXGI_FORMAT_R16G16_FLOAT;
	resDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
	alloc = device->GetResourceAllocationInfo(1, 1, &resDesc);
	desc.size = alloc.SizeInBytes;

	ptr = s_resourceManager->createTextureRtDs(L"rsmFilteredNormal", desc);
	if (ptr == nullptr)
		return false;

	return true;
}

bool RenderPassFilterRSM::loadShaders()
{
	if (!s_shaderManager->loadShader("ShadowMapFilterVS.cso", L"../../lib/ShadowMapFilterVS.cso", d3d::ShaderType::Vertex))
		return false;

	if (!s_shaderManager->loadShader("ShadowMapFilterGS.cso", L"../../lib/ShadowMapFilterGS.cso", d3d::ShaderType::Geometry))
		return false;

	if (!s_shaderManager->loadShader("ShadowMapFilterPS.cso", L"../../lib/ShadowMapFilterPS.cso", d3d::ShaderType::Pixel))
		return false;

	return true;
}

bool RenderPassFilterRSM::createRootSignature(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangePS[2];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0, 0, 0);
	rangePS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 4);

	CD3DX12_ROOT_PARAMETER rootParameters[2];
	rootParameters[0].InitAsDescriptorTable(2, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
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

bool RenderPassFilterRSM::createPipelineState(ID3D12Device* device)
{
	DXGI_FORMAT formats[2]=
	{
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_R16G16_FLOAT
	};

	d3d::PipelineStateDescInput inputDesc;
	inputDesc.inputLayout.pInputElementDescs = nullptr;
	inputDesc.inputLayout.NumElements = 0;
	inputDesc.depthEnabled = 0;
	inputDesc.rootSignature = m_rootSignature.get();
	inputDesc.shaderDesc.vs = s_shaderManager->getShader("ShadowMapFilterVS.cso");
	inputDesc.shaderDesc.gs = s_shaderManager->getShader("ShadowMapFilterGS.cso");
	inputDesc.shaderDesc.ps = s_shaderManager->getShader("ShadowMapFilterPS.cso");
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	inputDesc.rtvCount = 2;
	inputDesc.rtvFormats = formats;

	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);

	return d3d::PipelineState::create(desc, &m_pipelineState, device);
}

bool RenderPassFilterRSM::createSrvHeap(ID3D12Device* device)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 4;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	if (!m_srvHeap.create(desc, device))
		return false;

	return true;
}

bool RenderPassFilterRSM::createRtvHeap(ID3D12Device* device)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 2;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	if (!m_rtvHeap.create(desc, device))
		return false;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Texture2DArray.ArraySize = 10 * 6;
	rtvDesc.Texture2DArray.FirstArraySlice = 0;
	rtvDesc.Texture2DArray.MipSlice = 0;
	rtvDesc.Texture2DArray.PlaneSlice = 0;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;

	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	auto rsmFilteredColor = s_resourceManager->getTextureRtDs(L"rsmFilteredColor");

	auto handle = m_rtvHeap.getHandleCpu();
	device->CreateRenderTargetView(rsmFilteredColor->get(), &rtvDesc, handle);

	rtvDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
	auto rsmFilteredNormal = s_resourceManager->getTextureRtDs(L"rsmFilteredNormal");

	handle.offset(1);
	device->CreateRenderTargetView(rsmFilteredNormal->get(), &rtvDesc, handle);

	return true;
}

void RenderPassFilterRSM::createViews()
{

}

bool RenderPassFilterRSM::initialize(ID3D12Device* device, void* p)
{
	if (!loadShaders())
	{
		return false;
	}

	if (!createRootSignature(device))
	{
		return false;
	}

	if (!createPipelineState(device))
	{
		return false;
	}

	if (!createSrvHeap(device))
	{
		return false;
	}

	if (!createRtvHeap(device))
		return false;

	if (!m_commandList.create(device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_allocator->get(), m_pipelineState.get()))
	{
		return false;
	}

	createViews();

	return true;
}

void RenderPassFilterRSM::shutdown()
{

}

void RenderPassFilterRSM::submitCommands(Graphics::CommandQueue* queue)
{
	const u32 resolution = 256;
	const f32 clearColor[4] = {0, 0, 0, 0};

	if (m_lightCount != 0)
	{
		D3D12_VIEWPORT viewport;
		viewport.Height = (f32)resolution;
		viewport.Width = (f32)resolution;
		viewport.MaxDepth = 1.0f;
		viewport.MinDepth = 0.0f;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;

		D3D12_RECT rectScissor;
		rectScissor.left = 0;
		rectScissor.top = 0;
		rectScissor.right = resolution;
		rectScissor.bottom = resolution;

		m_commandList->Reset(m_allocator->get(), m_pipelineState.get());

		m_commandList->RSSetViewports(1, &viewport);
		m_commandList->RSSetScissorRects(1, &rectScissor);

		auto rtvHandle = m_rtvHeap.getHandleCpu();

		D3D12_CPU_DESCRIPTOR_HANDLE handles[2];
		handles[0] = rtvHandle;
		rtvHandle.offset(1);
		handles[1] = rtvHandle;

		m_commandList->OMSetRenderTargets(2, handles, 0, nullptr);
		m_commandList->ClearRenderTargetView(handles[0], clearColor, 0, nullptr);
		m_commandList->ClearRenderTargetView(handles[1], clearColor, 0, nullptr);

		m_commandList->Close();

		queue->pushCommandList(&m_commandList);
	}
}