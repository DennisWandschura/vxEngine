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
#include "RenderPassCullLights.h"
#include "d3dx12.h"
#include "ShaderManager.h"
#include "ResourceManager.h"
#include "GpuLight.h"
#include "CommandAllocator.h"
#include "DownloadMananger.h"
#include "ResourceDesc.h"

RenderPassCullLights::RenderPassCullLights(d3d::CommandAllocator* allocator, DownloadManager* downloadManager)
	:m_allocator(allocator),
	m_downloadManager(downloadManager),
	m_cpuDst(nullptr),
	m_buildList(0),
	m_lightCount(0)
{

}

RenderPassCullLights::~RenderPassCullLights()
{

}

void RenderPassCullLights::getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device)
{
}

bool RenderPassCullLights::createData(ID3D12Device* device)
{

	return true;
}

bool RenderPassCullLights::loadShaders()
{
	if (!s_shaderManager->loadShader(L"CullLightsNewVS.cso"))
		return false;

	return true;
}

bool RenderPassCullLights::createRootSignature(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangeVS[3];
	rangeVS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0, 0);
	rangeVS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0, 0, 2);
	rangeVS[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, 4);

	CD3DX12_ROOT_PARAMETER rootParameters[2];
	rootParameters[0].InitAsDescriptorTable(3, rangeVS, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsConstants(1, 2, 0, D3D12_SHADER_VISIBILITY_VERTEX);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(2, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	return m_rootSignature.create(&rootSignatureDesc, device);
}

bool RenderPassCullLights::createPipelineState(ID3D12Device* device)
{
	d3d::PipelineStateDescInput inputDesc;
	inputDesc.rootSignature = m_rootSignature.get();
	inputDesc.shaderDesc.vs = s_shaderManager->getShader(L"CullLightsNewVS.cso");
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	inputDesc.rtvCount = 0;
	inputDesc.depthEnabled = 0;

	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);

	return d3d::PipelineState::create(desc, &m_pipelineState, device);
}

bool RenderPassCullLights::createViews(ID3D12Device* device)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 5;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (!m_rvHeap.create(desc, device))
		return false;

	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NumDescriptors = 1;
	if (!m_uavClearHeap.create(desc, device))
		return false;
	/*
	Texture2D<float> g_zBuffer : register(t0);
	StructuredBuffer<GpuLight> g_lights : register(t1);

	cbuffer CameraBuffer : register(b0)
	cbuffer CameraStaticBuffer : register(b1)

	RWStructuredBuffer<uint> g_visibleLights : register(u0);
	*/

	D3D12_SHADER_RESOURCE_VIEW_DESC zbufferDesc;
	zbufferDesc.Format = DXGI_FORMAT_R32_FLOAT;
	zbufferDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING ;
	zbufferDesc.Texture2D.MipLevels = 1;
	zbufferDesc.Texture2D.MostDetailedMip = 0;
	zbufferDesc.Texture2D.PlaneSlice = 0;
	zbufferDesc.Texture2D.ResourceMinLODClamp = 0;
	zbufferDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	auto handle = m_rvHeap.getHandleCpu();
	auto zBuffer = s_resourceManager->getTextureRtDs(L"zBuffer");
	device->CreateShaderResourceView(zBuffer->get(), &zbufferDesc, handle);

	auto lightBuffer = s_resourceManager->getBuffer(L"lightBuffer");
	auto lightBufferView = s_resourceManager->getShaderResourceView("lightBufferView");
	handle.offset(1);
	device->CreateShaderResourceView(lightBuffer->get(), lightBufferView, handle);

	auto cameraBufferView = s_resourceManager->getConstantBufferView("cameraBufferView");
	handle.offset(1);
	device->CreateConstantBufferView(cameraBufferView, handle);

	auto cameraStaticBufferView = s_resourceManager->getConstantBufferView("cameraStaticBufferView");
	handle.offset(1);
	device->CreateConstantBufferView(cameraStaticBufferView, handle);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	uavDesc.Buffer.NumElements = s_settings->m_gpuLightCount;
	uavDesc.Buffer.StructureByteStride = sizeof(u32);
	auto visibleLightIndicesBuffer = s_resourceManager->getBuffer(L"visibleLightIndicesBuffer");
	handle.offset(1);
	device->CreateUnorderedAccessView(visibleLightIndicesBuffer->get(), nullptr, &uavDesc, handle);

	device->CreateUnorderedAccessView(visibleLightIndicesBuffer->get(), nullptr, &uavDesc, m_uavClearHeap.getHandleCpu());

	return true;
}

bool RenderPassCullLights::initialize(ID3D12Device* device, void* p)
{
	if (!loadShaders())
		return false;

	if (!createRootSignature(device))
		return false;

	if (!createPipelineState(device))
		return false;

	if (!m_commandList.create(device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_allocator->get(), m_pipelineState.get()))
		return false;

	if (!createViews(device))
		return false;

	return true;
}

void RenderPassCullLights::shutdown()
{

}

void RenderPassCullLights::buildCommands()
{
	// visiblity test lights in frustum

	// download results

	// sort results by distance

	const f32 clearColor[4] = {0, 0, 0, 0};
	if (m_checkEvent.getStatus() == EventStatus::Running)
	{
		auto visibleLightIndicesBuffer = s_resourceManager->getBuffer(L"visibleLightIndicesBuffer");

		// zero result buffer

		m_commandList->Reset(m_allocator->get(), m_pipelineState.get());

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

		m_commandList->RSSetViewports(1, &viewport);
		m_commandList->RSSetScissorRects(1, &rectScissor);

		auto heap = m_rvHeap.get();
		m_commandList->SetDescriptorHeaps(1, &heap);

		auto zBuffer = s_resourceManager->getTextureRtDs(L"zBuffer");

		m_commandList->ClearUnorderedAccessViewUint(m_uavClearHeap->GetGPUDescriptorHandleForHeapStart(), m_uavClearHeap->GetCPUDescriptorHandleForHeapStart(), visibleLightIndicesBuffer->get(), (u32*)clearColor, 0, 0);
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(visibleLightIndicesBuffer->get()));
		zBuffer->barrierTransition(m_commandList.get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		// visiblity test lights in frustum

		m_commandList->SetPipelineState(m_pipelineState.get());

		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		m_commandList->SetGraphicsRootSignature(m_rootSignature.get());
		m_commandList->SetGraphicsRootDescriptorTable(0, heap->GetGPUDescriptorHandleForHeapStart());
		m_commandList->SetGraphicsRoot32BitConstant(1, m_lightCount, 0);
		m_commandList->DrawInstanced(s_resolution.x, s_resolution.y, 0, 0);

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(visibleLightIndicesBuffer->get()));

		m_commandList->Close();
		m_buildList = 1;

		auto dlSize = sizeof(u32) * m_lightCount;
		m_downloadManager->pushDownloadBuffer(m_cpuDst, dlSize, visibleLightIndicesBuffer, 0, m_downloadEvent);

		m_downloadEvent.setStatus(EventStatus::Running);
		m_checkEvent.setStatus(EventStatus::Complete);
	}
}

void RenderPassCullLights::submitCommands(Graphics::CommandQueue* queue)
{
	if (m_buildList != 0)
	{
		queue->pushCommandList(&m_commandList);
		m_buildList = 0;
	}
}

void RenderPassCullLights::setEvent(const Event &checkEvt, const Event &dlEvent, u8* cpuDst)
{
	m_checkEvent = checkEvt;
	m_downloadEvent = dlEvent;
	m_cpuDst = cpuDst;
}