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

RenderPassCullLights::RenderPassCullLights(d3d::CommandAllocator* allocator, DownloadManager* downloadManager)
	:m_allocator(allocator),
	m_downloadManager(downloadManager),
	m_cpuDst(nullptr)
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
	if (!s_shaderManager->loadShader(L"CullLightsVS.cso"))
		return false;

	if (!s_shaderManager->loadShader(L"CullLightsGS.cso"))
		return false;

	if (!s_shaderManager->loadShader(L"CullLightsPS.cso"))
		return false;

	if (!s_shaderManager->loadShader(L"ZeroLightsVS.cso"))
		return false;

	return true;
}

bool RenderPassCullLights::createRootSignature(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangeVS[1];
	rangeVS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, 1);

	CD3DX12_DESCRIPTOR_RANGE rangeGS[1];
	rangeGS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 2);

	CD3DX12_DESCRIPTOR_RANGE rangePS[1];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, 0);

	CD3DX12_ROOT_PARAMETER rootParameters[3];
	rootParameters[0].InitAsDescriptorTable(1, rangeVS, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsDescriptorTable(1, rangeGS, D3D12_SHADER_VISIBILITY_GEOMETRY);
	rootParameters[2].InitAsDescriptorTable(1, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(3, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	return m_rootSignature.create(&rootSignatureDesc, device);
}

bool RenderPassCullLights::createRootSignatureZero(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangeVS[1];
	rangeVS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, 0);

	CD3DX12_ROOT_PARAMETER rootParameters[1];
	rootParameters[0].InitAsDescriptorTable(1, rangeVS, D3D12_SHADER_VISIBILITY_VERTEX);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(1, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob* blob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	auto hresult = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &errorBlob);
	if (hresult != 0)
		return false;

	hresult = device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(m_rootSignatureZeroLights.getAddressOf()));
	if (hresult != 0)
		return false;

	return true;
}

bool RenderPassCullLights::createPipelineState(ID3D12Device* device)
{
	d3d::PipelineStateDescInput inputDesc;
	inputDesc.rootSignature = m_rootSignature.get();
	inputDesc.shaderDesc.vs = s_shaderManager->getShader(L"CullLightsVS.cso");
	inputDesc.shaderDesc.gs = s_shaderManager->getShader(L"CullLightsGS.cso");
	inputDesc.shaderDesc.ps = s_shaderManager->getShader(L"CullLightsPS.cso");
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	inputDesc.rtvCount = 0;
	inputDesc.dsvFormat = DXGI_FORMAT_D32_FLOAT;
	inputDesc.depthEnabled = 1;
	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);

	desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;

	return d3d::PipelineState::create(desc, &m_pipelineState, device);
}

bool RenderPassCullLights::createPipelineStateZero(ID3D12Device* device)
{
	auto vsShader = s_shaderManager->getShader(L"ZeroLightsVS.cso");

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { nullptr, 0 };
	psoDesc.pRootSignature = m_rootSignatureZeroLights.get();
	psoDesc.VS = { reinterpret_cast<UINT8*>(vsShader->GetBufferPointer()), vsShader->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.FrontCounterClockwise = 1;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = 0;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	psoDesc.NumRenderTargets = 0;
	psoDesc.SampleDesc.Count = 1;

	auto hresult = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipelineStateZeroLights.getAddressOf()));
	if (hresult != 0)
		return false;

	return true;
}

bool RenderPassCullLights::createViews(ID3D12Device* device)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 3;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (!m_rvHeap.create(desc, device))
		return false;

	
	auto visibleLightIndicesBuffer = s_resourceManager->getBuffer(L"visibleLightIndicesBuffer");


	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	uavDesc.Buffer.NumElements = s_settings->m_gpuLightCount;
	uavDesc.Buffer.StructureByteStride = sizeof(u32);

	auto handle = m_rvHeap.getHandleCpu();
	device->CreateUnorderedAccessView(visibleLightIndicesBuffer->get(), nullptr, &uavDesc, handle);

	auto lightBuffer = s_resourceManager->getBuffer(L"lightBuffer");
	auto lightBufferView = s_resourceManager->getShaderResourceView("lightBufferView");

	handle.offset(1);
	device->CreateShaderResourceView(lightBuffer->get(), lightBufferView, handle);
	// srv

	auto cameraBufferView = s_resourceManager->getConstantBufferView("cameraBufferView");
	handle.offset(1);
	// cbv
	device->CreateConstantBufferView(cameraBufferView, handle);

	return true;
}

bool RenderPassCullLights::createRtvDsv(ID3D12Device* device)
{
	auto gbufferDepth = s_resourceManager->getTextureRtDs(L"gbufferDepth");

	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	if (device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_dsvHeap.getAddressOf())) != 0)
		return false;

	D3D12_DEPTH_STENCIL_VIEW_DESC depthViewDesc;
	depthViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
	depthViewDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
	depthViewDesc.Texture2DArray.ArraySize = 1;
	depthViewDesc.Texture2DArray.FirstArraySlice = 0;
	depthViewDesc.Texture2DArray.MipSlice = 0;
	device->CreateDepthStencilView(gbufferDepth->get(), &depthViewDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

	return true;
}

bool RenderPassCullLights::initialize(ID3D12Device* device, void* p)
{
	if (!loadShaders())
		return false;

	if (!createRootSignature(device))
		return false;

	if(!createRootSignatureZero(device))
		return false;

	if (!createPipelineState(device))
		return false;

	if (!createPipelineStateZero(device))
		return false;

	if (!m_commandList.create(device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_allocator->get(), m_pipelineState.get()))
		return false;

	if (!createRtvDsv(device))
		return false;

	if (!createViews(device))
		return false;

	return true;
}

void RenderPassCullLights::shutdown()
{

}

void RenderPassCullLights::submitCommands(Graphics::CommandQueue* queue)
{
	// visiblity test lights in frustum

	// download results

	// sort results by distance

	if (m_checkEvent.getStatus() == EventStatus::Running)
	{
		auto visibleLightIndicesBuffer = s_resourceManager->getBuffer(L"visibleLightIndicesBuffer");
		auto gbufferDepth = s_resourceManager->getTextureRtDs(L"gbufferDepth");

		// zero result buffer

		m_commandList->Reset(m_allocator->get(), m_pipelineStateZeroLights.get());

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
		m_commandList->SetGraphicsRootSignature(m_rootSignatureZeroLights.get());

		m_commandList->SetGraphicsRootDescriptorTable(0, heap->GetGPUDescriptorHandleForHeapStart());

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(visibleLightIndicesBuffer->get()));

		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		m_commandList->DrawInstanced(m_lightCount, 1, 0, 0);

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(visibleLightIndicesBuffer->get()));

		// visiblity test lights in frustum
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gbufferDepth->get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ));
		m_commandList->SetPipelineState(m_pipelineState.get());

		auto dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
		m_commandList->OMSetRenderTargets(0, nullptr, 0, &dsvHandle);

		m_commandList->SetGraphicsRootSignature(m_rootSignature.get());
		m_commandList->SetGraphicsRootDescriptorTable(0, heap->GetGPUDescriptorHandleForHeapStart());
		m_commandList->SetGraphicsRootDescriptorTable(1, heap->GetGPUDescriptorHandleForHeapStart());
		m_commandList->SetGraphicsRootDescriptorTable(2, heap->GetGPUDescriptorHandleForHeapStart());

		m_commandList->DrawInstanced(m_lightCount, 1, 0, 0);

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gbufferDepth->get(), D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(visibleLightIndicesBuffer->get()));

		m_commandList->Close();

		queue->pushCommandList(&m_commandList);

		auto dlSize = sizeof(u32) * m_lightCount;
		m_downloadManager->pushDownloadBuffer(m_cpuDst, dlSize, visibleLightIndicesBuffer, 0, m_downloadEvent);

		m_downloadEvent.setStatus(EventStatus::Running);
		m_checkEvent.setStatus(EventStatus::Complete);
	}
}

void RenderPassCullLights::setEvent(const Event &checkEvt, const Event &dlEvent, u8* cpuDst)
{
	m_checkEvent = checkEvt;
	m_downloadEvent = dlEvent;
	m_cpuDst = cpuDst;
}