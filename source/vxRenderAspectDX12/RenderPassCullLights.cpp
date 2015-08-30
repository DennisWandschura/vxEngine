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

RenderPassCullLights::RenderPassCullLights(ID3D12CommandAllocator* allocator)
	:m_allocator(allocator)
{

}

RenderPassCullLights::~RenderPassCullLights()
{

}

void RenderPassCullLights::getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device)
{
	D3D12_RESOURCE_DESC resDesc;
	resDesc.Alignment = 64 KBYTE;
	resDesc.DepthOrArraySize = 1;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Height = s_resolution.y;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Width = s_resolution.x;

	auto allocInfo = device->GetResourceAllocationInfo(1, 1, &resDesc);

	*heapSizeRtDs += allocInfo.SizeInBytes;

	const u32 lightCountGpu = s_settings->m_gpuLightCount + 1;
	auto lightBufferDstSize = d3d::getAlignedSize(sizeof(GpuLight) * lightCountGpu, 64llu KBYTE);
	auto lightIndexBufferSize = d3d::getAlignedSize(sizeof(u32) * lightCountGpu, 64llu KBYTE);

	*heapSizeBuffer += lightBufferDstSize + lightIndexBufferSize;
}

bool RenderPassCullLights::loadShaders()
{
	if (!s_shaderManager->loadShader("CullLightsVS.cso", L"../../lib/CullLightsVS.cso", d3d::ShaderType::Vertex))
		return false;

	if (!s_shaderManager->loadShader("CullLightsGS.cso", L"../../lib/CullLightsGS.cso", d3d::ShaderType::Geometry))
		return false;

	if (!s_shaderManager->loadShader("CullLightsPS.cso", L"../../lib/CullLightsPS.cso", d3d::ShaderType::Pixel))
		return false;

	if (!s_shaderManager->loadShader("ZeroLightsVS.cso", L"../../lib/ZeroLightsVS.cso", d3d::ShaderType::Vertex))
		return false;

	return true;
}

bool RenderPassCullLights::createRootSignature(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangeVS[1];
	rangeVS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, 0);

	CD3DX12_DESCRIPTOR_RANGE rangeGS[1];
	rangeGS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 1);

	CD3DX12_DESCRIPTOR_RANGE rangePS[1];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, 2);

	CD3DX12_ROOT_PARAMETER rootParameters[3];
	rootParameters[0].InitAsDescriptorTable(1, rangeVS, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsDescriptorTable(1, rangeGS, D3D12_SHADER_VISIBILITY_GEOMETRY);
	rootParameters[2].InitAsDescriptorTable(1, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(3, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

bool RenderPassCullLights::createRootSignatureZero(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangeVS[1];
	rangeVS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 2, 0, 0, 2);

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
	auto vsShader = s_shaderManager->getShader("CullLightsVS.cso");
	auto gsShader = s_shaderManager->getShader("CullLightsGS.cso");
	auto psShader = s_shaderManager->getShader("CullLightsPS.cso");

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
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	psoDesc.NumRenderTargets = 0;
	//psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	auto hresult = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipelineState.getAddressOf()));
	if (hresult != 0)
		return false;

	return true;
}

bool RenderPassCullLights::createPipelineStateZero(ID3D12Device* device)
{
	auto vsShader = s_shaderManager->getShader("ZeroLightsVS.cso");

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

bool RenderPassCullLights::createTexture(ID3D12Device* device)
{
	D3D12_RESOURCE_DESC resDesc;
	resDesc.Alignment = 64 KBYTE;
	resDesc.DepthOrArraySize = 1;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Height = s_resolution.y;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Width = s_resolution.x;

	auto allocInfo = device->GetResourceAllocationInfo(1, 1, &resDesc);

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	clearValue.Color[0] = 0;
	clearValue.Color[1] = 0;
	clearValue.Color[2] = 0;
	clearValue.Color[3] = 1;

	CreateResourceDesc desc;
	desc.clearValue = &clearValue;
	desc.resDesc = &resDesc;
	desc.size = allocInfo.SizeInBytes;
	desc.state = D3D12_RESOURCE_STATE_RENDER_TARGET;
	auto ptr = s_resourceManager->createTextureRtDs(L"lightTexture", desc);
	if (ptr == nullptr)
		return false;

	return true;
}

bool RenderPassCullLights::createBuffer()
{
	auto indexBuffer = s_resourceManager->createBuffer(L"visibleLightIndexBuffer", 64 KBYTE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	if (indexBuffer == nullptr)
		return false;

	auto lightBufferDst = s_resourceManager->createBuffer(L"lightBufferDst", 64 KBYTE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	if (lightBufferDst == nullptr)
		return false;

	return true;
}

bool RenderPassCullLights::createViews(ID3D12Device* device)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 4;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (!m_rvHeap.create(desc, device))
		return false;

	auto lightBuffer = s_resourceManager->getBuffer(L"lightBuffer");
	auto lightBufferDst = s_resourceManager->getBuffer(L"lightBufferDst");
	auto lightBufferView = s_resourceManager->getShaderResourceView("lightBufferView");
	auto cameraBufferView = s_resourceManager->getConstantBufferView("cameraBufferView");
	auto visibleLightIndexBuffer = s_resourceManager->getBuffer(L"visibleLightIndexBuffer");

	auto handle = m_rvHeap.getHandleCpu();
	device->CreateShaderResourceView(lightBuffer, lightBufferView, handle);

	handle.offset(1);
	device->CreateConstantBufferView(cameraBufferView, handle);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	uavDesc.Buffer.NumElements = s_settings->m_gpuLightCount + 1;
	uavDesc.Buffer.StructureByteStride = sizeof(u32);

	handle.offset(1);
	device->CreateUnorderedAccessView(visibleLightIndexBuffer, nullptr, &uavDesc, handle);

	uavDesc.Buffer.StructureByteStride = sizeof(GpuLight);
	handle.offset(1);
	device->CreateUnorderedAccessView(lightBufferDst, nullptr, &uavDesc, handle);

	return true;
}

bool RenderPassCullLights::createRtvDsv(ID3D12Device* device)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	if (device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_rtvHeap.getAddressOf())) != 0)
		return false;
	
	auto lightTexture = s_resourceManager->getTextureRtDs(L"lightTexture");
	auto gbufferDepth = s_resourceManager->getTextureRtDs(L"gbufferDepth");

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;
	device->CreateRenderTargetView(lightTexture, &rtvDesc, m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

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
	device->CreateDepthStencilView(gbufferDepth, &depthViewDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

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

	if (!createTexture(device))
		return false;

	if (!createBuffer())
		return false;

	if (!createViews(device))
		return false;

	if (!createRtvDsv(device))
		return false;

	if (!m_commandList.create(device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_allocator, m_pipelineState.get()))
		return false;

	return true;
}

void RenderPassCullLights::shutdown()
{

}

void RenderPassCullLights::submitCommands(ID3D12CommandList** list, u32* index)
{
	if (m_lightCount != 0)
	{
		auto lightBuffer = s_resourceManager->getBuffer(L"lightBuffer");
		auto visibleLightIndexBuffer = s_resourceManager->getBuffer(L"visibleLightIndexBuffer");

		const f32 clearColor[4] = { 0, 0, 0, 1 };
		auto resolution = s_resolution;

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

		m_commandList->Reset(m_allocator, nullptr);

		m_commandList->RSSetScissorRects(1, &rectScissor);
		m_commandList->RSSetViewports(1, &viewPort);

		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		auto heap = m_rvHeap.get();
		m_commandList->SetDescriptorHeaps(1, &heap);

		{
			m_commandList->SetPipelineState(m_pipelineStateZeroLights.get());

			m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(visibleLightIndexBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
			// zero light buffers

			m_commandList->SetGraphicsRootSignature(m_rootSignatureZeroLights.get());
			m_commandList->SetGraphicsRootDescriptorTable(0, m_rvHeap->GetGPUDescriptorHandleForHeapStart());

			m_commandList->DrawInstanced(m_lightCount + 1, 1, 0, 0);

			m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(visibleLightIndexBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		}

		m_commandList->SetPipelineState(m_pipelineState.get());
		m_commandList->SetGraphicsRootSignature(m_rootSignature.get());
		m_commandList->SetGraphicsRootDescriptorTable(0, m_rvHeap->GetGPUDescriptorHandleForHeapStart());
		m_commandList->SetGraphicsRootDescriptorTable(1, m_rvHeap->GetGPUDescriptorHandleForHeapStart());

		auto dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
		m_commandList->OMSetRenderTargets(0, nullptr, 0, &dsvHandle);

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(lightBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

		m_commandList->DrawInstanced(m_lightCount, 1, 0, 0);

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(lightBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		m_commandList->Close();

		list[*index] = m_commandList.get();
		++(*index);
	}
}