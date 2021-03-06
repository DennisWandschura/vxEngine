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

#include "RenderPassZBuffer.h"
#include <d3d12.h>
#include "ShaderManager.h"
#include "d3dx12.h"
#include "ResourceManager.h"
#include "CommandAllocator.h"
#include "GpuCameraBufferData.h"
#include "GpuProfiler.h"

const u32 g_zBufferMaxMipLevel = 5u;

RenderPassZBuffer::RenderPassZBuffer()
	:RenderPass(),
	m_descriptorHeapRtv(),
	m_descriptorHeap()
{

}

RenderPassZBuffer::~RenderPassZBuffer()
{

}

void RenderPassZBuffer::getRequiredMemory(u64* heapSizeBuffer, u32* bufferCount, u64* heapSizeTexture, u32* textureCount, u64* heapSizeRtDs, u32* rtDsCount, ID3D12Device* device)
{

}

bool RenderPassZBuffer::createData(ID3D12Device* device)
{
	return true;
}

bool RenderPassZBuffer::loadShaders(d3d::ShaderManager* shaderManager)
{
	if (!shaderManager->loadShader(L"DrawQuadVs.cso"))
		return false;

	if (!shaderManager->loadShader(L"DrawQuadGs.cso"))
		return false;

	if (!shaderManager->loadShader(L"CreateZBufferPS.cso"))
		return false;

	return true;
}

bool RenderPassZBuffer::createRootSignature(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangePS[2];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 0);
	rangePS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, 1);

	CD3DX12_ROOT_PARAMETER rootParameters[1];
	rootParameters[0].InitAsDescriptorTable(2, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
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

	return m_rootSignature.create(&rootSignatureDesc, device);
}

bool RenderPassZBuffer::createPipelineState(ID3D12Device* device, d3d::ShaderManager* shaderManager)
{
	DXGI_FORMAT rtvFormats[] = { DXGI_FORMAT_R32_FLOAT };

	d3d::PipelineStateDescInput inputDesc;
	inputDesc.rootSignature = m_rootSignature.get();
	inputDesc.depthEnabled = 0;
	inputDesc.shaderDesc.vs = s_shaderManager->getShader(L"DrawQuadVs.cso");
	inputDesc.shaderDesc.gs = s_shaderManager->getShader(L"DrawQuadGs.cso");
	inputDesc.shaderDesc.ps = s_shaderManager->getShader(L"CreateZBufferPS.cso");
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	inputDesc.rtvFormats = rtvFormats;
	inputDesc.rtvCount = 1;
	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);

	return d3d::PipelineState::create(desc, &m_pipelineState, device);
}

bool RenderPassZBuffer::createDescriptor(ID3D12Device* device, d3d::ResourceManager* resourceManager)
{
	auto cbufferDesc = resourceManager->getConstantBufferView("cameraStaticBufferView");
	auto gbufferDepthTexture = resourceManager->getTextureRtDs(L"gbufferDepth");
	auto zBuffer = resourceManager->getTextureRtDs(L"zBuffer");
	auto zBufferDesc = zBuffer->GetDesc();

	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 2;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	D3D12_SHADER_RESOURCE_VIEW_DESC depthDesc;
	depthDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
	depthDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	depthDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	depthDesc.Texture2D.MipLevels = 1;
	depthDesc.Texture2D.MostDetailedMip = 0;
	depthDesc.Texture2D.PlaneSlice = 0;
	depthDesc.Texture2D.ResourceMinLODClamp = 0;

	if (!m_descriptorHeap.create(desc, device))
		return false;

	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NumDescriptors = 2;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	if (!m_descriptorHeapRtv.create(desc, device))
		return false;
	
	auto handle = m_descriptorHeap.getHandleCpu();
	device->CreateConstantBufferView(cbufferDesc, handle);

	handle.offset(1);
	device->CreateShaderResourceView(gbufferDepthTexture->get(), &depthDesc, handle);

	auto handleRtv = m_descriptorHeapRtv.getHandleCpu();
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = zBufferDesc.Format;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;
	device->CreateRenderTargetView(zBuffer->get(), &rtvDesc, handleRtv);

	return true;
}

bool RenderPassZBuffer::initialize(ID3D12Device* device, d3d::CommandAllocator* allocators, u32 frameCount)
{
	if (!loadShaders(s_shaderManager))
		return false;

	if (!createRootSignature(device))
		return false;

	if (!createPipelineState(device, s_shaderManager))
		return false;

	if (!createDescriptor(device, s_resourceManager))
		return false;

	if (!createCommandLists(device, D3D12_COMMAND_LIST_TYPE_DIRECT, allocators, frameCount))
		return false;

	return true;
}

void RenderPassZBuffer::shutdown()
{
	m_descriptorHeap.destroy();
}

void RenderPassZBuffer::buildCommands(d3d::CommandAllocator* currentAllocator, u32 frameIndex)
{
	auto gbufferDepthTexture = s_resourceManager->getTextureRtDs(L"gbufferDepth");
	auto zBuffer = s_resourceManager->getTextureRtDs(L"zBuffer");

	const f32 clearColor[4] = { 1.0f, 0.0f, 0, 0 };

	auto &currentCommandList = m_commandLists[frameIndex];
	m_currentCommandList = &currentCommandList;

	currentCommandList->Reset(currentAllocator->get(), m_pipelineState.get());

	s_gpuProfiler->queryBegin("zbuffer", &currentCommandList);

	D3D12_VIEWPORT viewport;
	viewport.Height = (f32)s_resolution.y;
	viewport.Width = (f32)s_resolution.x;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	D3D12_RECT rectScissor;
	rectScissor.left = 0;
	rectScissor.top = 0;
	rectScissor.right = s_resolution.x;
	rectScissor.bottom = s_resolution.y;

	currentCommandList->RSSetViewports(1, &viewport);
	currentCommandList->RSSetScissorRects(1, &rectScissor);

	currentCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(zBuffer->get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
	currentCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gbufferDepthTexture->get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	auto rtvHandle = m_descriptorHeapRtv.getHandleCpu();
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[1];
	rtvHandles[0] = rtvHandle;
	rtvHandle.offset(1);

	currentCommandList->OMSetRenderTargets(1, rtvHandles, FALSE, nullptr);
	currentCommandList->ClearRenderTargetView(rtvHandles[0], clearColor, 0, nullptr);

	currentCommandList->SetGraphicsRootSignature(m_rootSignature.get());

	auto descriptor = m_descriptorHeap.get();
	currentCommandList->SetDescriptorHeaps(1, &descriptor);
	currentCommandList->SetGraphicsRootDescriptorTable(0, m_descriptorHeap->GetGPUDescriptorHandleForHeapStart());

	currentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	currentCommandList->DrawInstanced(1, 1, 0, 0);

	currentCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gbufferDepthTexture->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE));
	currentCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(zBuffer->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));

	s_gpuProfiler->queryEnd(&currentCommandList);

	currentCommandList->Close();
}

void RenderPassZBuffer::submitCommands(Graphics::CommandQueue* queue)
{
	queue->pushCommandList(m_currentCommandList);
}