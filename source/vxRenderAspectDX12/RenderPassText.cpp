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

#include "RenderPassText.h"
#include "d3dx12.h"
#include "ShaderManager.h"
#include "CommandAllocator.h"
#include "ResourceDesc.h"
#include "ResourceManager.h"
#include "ResourceView.h"
#include "Device.h"

RenderPassText::RenderPassText(d3d::CommandAllocator* alloc, d3d::Device* device)
	:m_commandList(),
	m_allocator(alloc),
	m_indexCount(0),
	m_buildList(0),
	m_device(device)
{

}

RenderPassText::~RenderPassText()
{

}

void RenderPassText::getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device)
{
	auto resDesc = d3d::ResourceDesc::getDescTexture2D(s_resolution, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
	auto allocInfo = device->GetResourceAllocationInfo(1, 1, &resDesc);

	*heapSizeRtDs += allocInfo.SizeInBytes;
}

bool RenderPassText::createData(ID3D12Device* device)
{
	auto resDesc = d3d::ResourceDesc::getDescTexture2D(s_resolution, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
	auto allocInfo = device->GetResourceAllocationInfo(1, 1, &resDesc);

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	CreateResourceDesc desc;
	desc.clearValue = &clearValue;
	desc.resDesc = &resDesc;
	desc.size = allocInfo.SizeInBytes;
	desc.state = D3D12_RESOURCE_STATE_RENDER_TARGET;

	auto textTexture = s_resourceManager->createTextureRtDs(L"textTexture", desc);
	if (textTexture == nullptr)
		return false;

	return true;
}

bool RenderPassText::createRootSignature(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangeVS[1];
	rangeVS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 0);

	CD3DX12_DESCRIPTOR_RANGE rangePS[1];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, 1);

	CD3DX12_ROOT_PARAMETER rootParameters[2];
	rootParameters[0].InitAsDescriptorTable(1, rangeVS, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsDescriptorTable(1, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);

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

bool RenderPassText::createPipelineState(ID3D12Device* device)
{
	/*
	struct Vertex
	{
	float3 position : POSITION0;
	float2 texCoords : TEXCOORDS0;
	float3 color : COLOR0;
	};
	*/
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	auto rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	d3d::PipelineStateDescInput inputDesc;
	inputDesc.rootSignature = m_rootSignature.get();
	inputDesc.depthEnabled = 0;
	inputDesc.shaderDesc.vs = s_shaderManager->getShader(L"TextVS.cso");
	inputDesc.shaderDesc.ps = s_shaderManager->getShader(L"TextPS.cso");
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	inputDesc.rtvFormats = &rtvFormat;
	inputDesc.rtvCount = 1;
	inputDesc.inputLayout = { inputLayout, _countof(inputLayout) };
	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);

	desc.BlendState.RenderTarget[0].BlendEnable = 1;
	desc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	desc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	desc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;

	return d3d::PipelineState::create(desc, &m_pipelineState, device);
}

bool RenderPassText::initialize(ID3D12Device* device, void* p)
{
	const wchar_t* files[2] =
	{
		L"TextVS.cso",
		L"TextPS.cso"
	};

	if (!loadShaders( files, 2))
		return false;

	if (!createRootSignature(device))
		return false;

	if (!createPipelineState(device))
		return false;

	if (!m_commandList.create(device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_allocator->get(), m_pipelineState.get()))
		return false;

	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 2;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	if (!m_rtvHeap.create(desc, device))
		return false;

	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NumDescriptors = 2;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (!m_srvHeap.create(desc, device))
		return false;

	auto rtvHandle = m_rtvHeap.getHandleCpu();
	for (u32 i = 0; i < 2; ++i)
	{
		m_device->getBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.Texture2D.MipSlice = 0,
		rtvDesc.Texture2D.PlaneSlice = 0;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		device->CreateRenderTargetView(m_renderTargets[i], &rtvDesc, rtvHandle);
		rtvHandle.offset(1);
	}

	auto cameraStaticBufferView = s_resourceManager->getConstantBufferView("cameraStaticBufferView");
	auto handle = m_srvHeap.getHandleCpu();
	device->CreateConstantBufferView(cameraStaticBufferView, handle);

	auto fontTexture = s_resourceManager->getTexture(L"fontTexture");
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp=0;
	handle.offset(1);
	device->CreateShaderResourceView(fontTexture->get(), &srvDesc, handle);

	return true;
}

void RenderPassText::shutdown()
{

}

void RenderPassText::buildCommands()
{
	if (m_indexCount != 0)
	{
		auto textVbv = s_resourceManager->getResourceView("textVbv");
		auto textIbv = s_resourceManager->getResourceView("textIbv");
		m_commandList->Reset(m_allocator->get(), m_pipelineState.get());

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

		auto currentBuffer = m_device->getCurrentBackBufferIndex();

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[currentBuffer], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		m_commandList->RSSetViewports(1, &viewPort);
		m_commandList->RSSetScissorRects(1, &rectScissor);

		auto rtvHandle = m_rtvHeap.getHandleCpu();
		rtvHandle.offset(currentBuffer);

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[1];
		rtvHandles[0] = rtvHandle;

		m_commandList->OMSetRenderTargets(1, rtvHandles, 0, nullptr);
		//m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

		auto srvHeap = m_srvHeap.get();
		m_commandList->SetDescriptorHeaps(1, &srvHeap);
		m_commandList->SetGraphicsRootSignature(m_rootSignature.get());
		m_commandList->SetGraphicsRootDescriptorTable(0, srvHeap->GetGPUDescriptorHandleForHeapStart());
		m_commandList->SetGraphicsRootDescriptorTable(1, srvHeap->GetGPUDescriptorHandleForHeapStart());

		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_commandList->IASetVertexBuffers(0, 1, &textVbv->vbv);
		m_commandList->IASetIndexBuffer(&textIbv->ibv);

		m_commandList->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[currentBuffer], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		m_commandList->Close();
		m_buildList = 1;
	}
}

void RenderPassText::submitCommands(Graphics::CommandQueue* queue)
{
	if (m_buildList != 0)
	{
		queue->pushCommandList(&m_commandList);
		m_buildList = 0;
	}
}