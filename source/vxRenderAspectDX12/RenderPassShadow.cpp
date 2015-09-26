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

#include "RenderPassShadow.h"
#include "GpuShadowTransform.h"
#include "ResourceManager.h"
#include "ShaderManager.h"
#include "d3dx12.h"
#include "DrawIndexedIndirectCommand.h"
#include "UploadManager.h"
#include "ResourceView.h"
#include "CommandAllocator.h"
#include "GpuVoxel.h"

namespace RenderPassShadowCpp
{
	enum Textures { TextureDepth, TextureZDepth, TextureIntensity, TextureNormal, TextureCount };

	void getDescription(D3D12_RESOURCE_DESC(&resDesc)[TextureCount], u32 resolution, u32 count)
	{
		resDesc[RenderPassShadowCpp::TextureDepth].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resDesc[RenderPassShadowCpp::TextureDepth].Alignment = 64 KBYTE;
		resDesc[RenderPassShadowCpp::TextureDepth].Width = resolution;
		resDesc[RenderPassShadowCpp::TextureDepth].Height = resolution;
		resDesc[RenderPassShadowCpp::TextureDepth].DepthOrArraySize = 6 * count;
		resDesc[RenderPassShadowCpp::TextureDepth].MipLevels = 1;
		resDesc[RenderPassShadowCpp::TextureDepth].Format = DXGI_FORMAT_R32_TYPELESS;
		resDesc[RenderPassShadowCpp::TextureDepth].SampleDesc.Count = 1;
		resDesc[RenderPassShadowCpp::TextureDepth].SampleDesc.Quality = 0;
		resDesc[RenderPassShadowCpp::TextureDepth].Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resDesc[RenderPassShadowCpp::TextureDepth].Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		resDesc[RenderPassShadowCpp::TextureZDepth] = resDesc[RenderPassShadowCpp::TextureDepth];
		resDesc[RenderPassShadowCpp::TextureZDepth].Format = DXGI_FORMAT_R32G32_FLOAT;
		resDesc[RenderPassShadowCpp::TextureZDepth].Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		resDesc[RenderPassShadowCpp::TextureIntensity] = resDesc[RenderPassShadowCpp::TextureZDepth];
		resDesc[RenderPassShadowCpp::TextureIntensity].Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		resDesc[RenderPassShadowCpp::TextureNormal] = resDesc[RenderPassShadowCpp::TextureIntensity];
		resDesc[RenderPassShadowCpp::TextureNormal].Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	}
}

RenderPassShadow::RenderPassShadow(d3d::CommandAllocator* alloc, DrawIndexedIndirectCommand* drawCmd)
	:RenderPassLight(),
	m_cmdAlloc(alloc),
	m_drawCmd(drawCmd),
	m_buildList(0)
{

}

RenderPassShadow::~RenderPassShadow()
{

}

void RenderPassShadow::getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device)
{
	D3D12_RESOURCE_DESC resDescHigh[RenderPassShadowCpp::TextureCount];
	RenderPassShadowCpp::getDescription(resDescHigh, s_settings->m_shadowDim, s_settings->m_shadowCastingLightCount);

	auto infoHigh = device->GetResourceAllocationInfo(1, RenderPassShadowCpp::TextureCount, resDescHigh);

	*heapSizeRtDs += infoHigh.SizeInBytes;
}

bool RenderPassShadow::createData(ID3D12Device* device)
{
	D3D12_RESOURCE_DESC resDescHigh[RenderPassShadowCpp::TextureCount];
	RenderPassShadowCpp::getDescription(resDescHigh, s_settings->m_shadowDim, s_settings->m_shadowCastingLightCount);

	D3D12_CLEAR_VALUE clearValues[RenderPassShadowCpp::TextureCount];
	memset(clearValues, 0, sizeof(clearValues));
	clearValues[RenderPassShadowCpp::TextureDepth].Format = DXGI_FORMAT_D32_FLOAT;
	clearValues[RenderPassShadowCpp::TextureDepth].DepthStencil.Depth = 1.f;
	clearValues[RenderPassShadowCpp::TextureDepth].DepthStencil.Stencil = 0;

	clearValues[RenderPassShadowCpp::TextureZDepth].Format = DXGI_FORMAT_R32G32_FLOAT;
	clearValues[RenderPassShadowCpp::TextureZDepth].Color[0] = 1.0f;

	clearValues[RenderPassShadowCpp::TextureIntensity].Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	clearValues[RenderPassShadowCpp::TextureNormal].Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

	const wchar_t* names[]=
	{
		L"shadowTextureDepth",
		L"shadowTextureLinear",
		L"shadowTextureColor",
		L"shadowTextureNormal"
	};

	D3D12_RESOURCE_STATES resStates[RenderPassShadowCpp::TextureCount];
	resStates[RenderPassShadowCpp::TextureDepth] = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	resStates[RenderPassShadowCpp::TextureZDepth] = D3D12_RESOURCE_STATE_RENDER_TARGET;
	resStates[RenderPassShadowCpp::TextureIntensity] = D3D12_RESOURCE_STATE_RENDER_TARGET;
	resStates[RenderPassShadowCpp::TextureNormal] = D3D12_RESOURCE_STATE_RENDER_TARGET;

	for (u32 i = 0; i < RenderPassShadowCpp::TextureCount; ++i)
	{
		auto info = device->GetResourceAllocationInfo(1, 1, &resDescHigh[i]);

		CreateResourceDesc desc;
		desc.clearValue = &clearValues[i];
		desc.resDesc = &resDescHigh[i];
		desc.size = info.SizeInBytes;
		desc.state = resStates[i];

		auto p = s_resourceManager->createTextureRtDs(names[i], desc);
		if (p == nullptr)
			return false;
	}

	return true;
}

bool RenderPassShadow::loadShaders()
{
	if (!s_shaderManager->loadShader(L"ShadowMapVS.cso"))
		return false;

	if (!s_shaderManager->loadShader(L"ShadowMapGS.cso"))
		return false;

	if (!s_shaderManager->loadShader(L"ShadowMapPS.cso"))
		return false;

	return true;
}

bool RenderPassShadow::createRootSignature(ID3D12Device* device)
{
	/*
	StructuredBuffer<TransformGpu> s_transforms : register(t0);
	StructuredBuffer<uint> s_materials : register(t1);
	StructuredBuffer<GpuLight> g_lights : register(t2)
	cbuffer CameraBuffer : register(b0)

	StructuredBuffer<ShadowTransform> shadowTransforms : register(t3);

	Texture2DArray g_srgb : register(t4);
	*/
	CD3DX12_DESCRIPTOR_RANGE rangeVS[2];
	rangeVS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 0, 0);
	rangeVS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 3);

	CD3DX12_DESCRIPTOR_RANGE rangeGS[1];
	rangeGS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0, 4);

	CD3DX12_DESCRIPTOR_RANGE rangePS[3];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 0, 5);
	rangePS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3, 0, 6);
	rangePS[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2, 0, 7);

	CD3DX12_ROOT_PARAMETER rootParameters[4];
	rootParameters[0].InitAsDescriptorTable(2, rangeVS, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsDescriptorTable(1, rangeGS, D3D12_SHADER_VISIBILITY_GEOMETRY);
	rootParameters[2].InitAsDescriptorTable(3, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[3].InitAsConstants(1, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX);

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
	rootSignatureDesc.Init(4, rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	return m_rootSignature.create(&rootSignatureDesc, device);
}

bool RenderPassShadow::createPipelineState(ID3D12Device* device)
{
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32_UINT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 }
	};

	DXGI_FORMAT format[3] = { DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_FLOAT };


	d3d::PipelineStateDescInput inputDesc;
	inputDesc.inputLayout.pInputElementDescs = inputLayout;
	inputDesc.inputLayout.NumElements = _countof(inputLayout);
	inputDesc.depthEnabled = 1;
	inputDesc.rootSignature = m_rootSignature.get();
	inputDesc.shaderDesc.vs = s_shaderManager->getShader(L"ShadowMapVS.cso");
	inputDesc.shaderDesc.gs = s_shaderManager->getShader(L"ShadowMapGS.cso");
	inputDesc.shaderDesc.ps = s_shaderManager->getShader(L"ShadowMapPS.cso");
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	inputDesc.dsvFormat = DXGI_FORMAT_D32_FLOAT;
	inputDesc.rtvCount = 3;
	inputDesc.rtvFormats = format;

	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);
	//desc.RasterizerState.SlopeScaledDepthBias = 2.5f;
	//desc.RasterizerState.DepthBias = 5;

	return d3d::PipelineState::create(desc, &m_pipelineState, device);
}

bool RenderPassShadow::createCommandList(ID3D12Device* device)
{
	return m_commandList.create(device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAlloc->get(), m_pipelineState.get());
}

bool RenderPassShadow::createRtvs(ID3D12Device* device, u32 shadowCastingLightCount)
{
	auto shadowTextureLinear = s_resourceManager->getTextureRtDs(L"shadowTextureLinear");
	auto shadowTextureColor = s_resourceManager->getTextureRtDs(L"shadowTextureColor");
	auto shadowTextureNormal = s_resourceManager->getTextureRtDs(L"shadowTextureNormal");

	const u32 renderTargetCount = 3;

	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 1;
	desc.NumDescriptors = renderTargetCount * shadowCastingLightCount;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	if (!m_heapRtv.create(desc, device))
		return false;

	auto handle = m_heapRtv.getHandleCpu();

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.ArraySize = 6;
	rtvDesc.Texture2DArray.MipSlice = 0;
	rtvDesc.Texture2DArray.PlaneSlice = 0;

	for (u32 i = 0; i < shadowCastingLightCount; ++i)
	{
		rtvDesc.Texture2DArray.FirstArraySlice = i * 6;

		rtvDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		device->CreateRenderTargetView(shadowTextureLinear->get(), &rtvDesc, handle);

		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		handle.offset(1);
		device->CreateRenderTargetView(shadowTextureColor->get(), &rtvDesc, handle);

		rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		handle.offset(1);
		device->CreateRenderTargetView(shadowTextureNormal->get(), &rtvDesc, handle);

		handle.offset(1);
	}

	return true;
}

bool RenderPassShadow::initialize(ID3D12Device* device, void* p)
{
	if (!loadShaders())
		return false;

	if (!createRootSignature(device))
		return false;

	if (!createPipelineState(device))
		return false;

	if (!createCommandList(device))
		return false;

	if (!createRtvs(device, s_settings->m_shadowCastingLightCount))
		return false;

	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 1;
	desc.NumDescriptors = s_settings->m_shadowCastingLightCount;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	if (!m_heapDsv.create(desc, device))
		return false;

	auto shadowTextureDepth = s_resourceManager->getTextureRtDs(L"shadowTextureDepth");

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.Texture2DArray.ArraySize = 6;
	dsvDesc.Texture2DArray.MipSlice = 0;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;

	auto handle = m_heapDsv.getHandleCpu();
	for (u32 i = 0; i < s_settings->m_shadowCastingLightCount; ++i)
	{
		dsvDesc.Texture2DArray.FirstArraySlice = i * 6;
		device->CreateDepthStencilView(shadowTextureDepth->get(), &dsvDesc, handle);

		handle.offset(1);
	}

	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NumDescriptors = 8;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (!m_heapSrv.create(desc, device))
		return false;

	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (!m_heapUav.create(desc, device))
		return false;
	

	/*
	StructuredBuffer<TransformGpu> s_transforms : register(t0);
	StructuredBuffer<uint> s_materials : register(t1);
	StructuredBuffer<GpuLight> g_lights : register(t2)
	cbuffer CameraBuffer : register(b0)

	StructuredBuffer<ShadowTransform> shadowTransforms : register(t3);

	Texture2DArray g_srgb : register(t4);
	*/

	auto srvHandle = m_heapSrv.getHandleCpu();

	auto transformBufferViewDesc = s_resourceManager->getShaderResourceView("transformBufferView");
	auto transformBuffer = s_resourceManager->getBuffer(L"transformBuffer");
	device->CreateShaderResourceView(transformBuffer->get(), transformBufferViewDesc, srvHandle);

	auto materialBuffer = s_resourceManager->getBuffer(L"materialBuffer");
	auto materialBufferViewDesc = s_resourceManager->getShaderResourceView("materialBufferView");
	srvHandle.offset(1);
	device->CreateShaderResourceView(materialBuffer->get(), materialBufferViewDesc, srvHandle);

	auto shadowCastingLightsBuffer = s_resourceManager->getBuffer(L"shadowCastingLightsBuffer");
	auto shadowCastingLightsBufferView = s_resourceManager->getShaderResourceView("shadowCastingLightsBufferView");
	srvHandle.offset(1);
	device->CreateShaderResourceView(shadowCastingLightsBuffer->get(), shadowCastingLightsBufferView, srvHandle);

	auto cameraBufferView = s_resourceManager->getConstantBufferView("cameraBufferView");
	srvHandle.offset(1);
	device->CreateConstantBufferView(cameraBufferView, srvHandle);

	auto shadowTransformBuffer = s_resourceManager->getBuffer(L"shadowTransformBuffer");
	auto shadowTransformBufferView = s_resourceManager->getShaderResourceView("shadowTransformBufferView");
	srvHandle.offset(1);
	device->CreateShaderResourceView(shadowTransformBuffer->get(), shadowTransformBufferView, srvHandle);

	auto srgbTextureViewDesc = s_resourceManager->getShaderResourceView("srgbTextureView");
	auto srgbTexture = s_resourceManager->getTexture(L"srgbTexture");
	srvHandle.offset(1);
	device->CreateShaderResourceView(srgbTexture->get(), srgbTextureViewDesc, srvHandle);


	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_R32_UINT;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	uavDesc.Texture3D.FirstWSlice = 0;
	uavDesc.Texture3D.MipSlice = 0;
	uavDesc.Texture3D.WSize = s_settings->m_lpvDim * 6;

	auto voxelTextureColorTmp = s_resourceManager->getTexture(L"voxelTextureColorTmp");
	srvHandle.offset(1);
	device->CreateUnorderedAccessView(voxelTextureColorTmp->get(), nullptr, &uavDesc, srvHandle);

	device->CreateUnorderedAccessView(voxelTextureColorTmp->get(), nullptr, &uavDesc, m_heapUav.getHandleCpu());

	auto voxelBuffer = s_resourceManager->getBuffer(L"voxelBuffer");
	D3D12_CONSTANT_BUFFER_VIEW_DESC voxelBufferDesc;
	voxelBufferDesc.BufferLocation = voxelBuffer->GetGPUVirtualAddress();
	voxelBufferDesc.SizeInBytes = d3d::AlignedSizeType<GpuVoxel, 1, 256>::size;

	srvHandle.offset(1);
	device->CreateConstantBufferView(&voxelBufferDesc, srvHandle);

	return true;
}

void RenderPassShadow::shutdown()
{

}

void RenderPassShadow::buildCommands()
{
	const f32 clearcolor[] = { 1.0f, 0, 0, 0 };
	const f32 clearcolor0[] = { 0.0f, 0, 0, 0 };
	const u32 clearValues[4] = { 0, 0, 0, 0 };

	auto count = m_drawCmd->getCount();
	if (count != 0 && m_visibleLightCount != 0)
	{
		D3D12_VIEWPORT viewPort;
		viewPort.Height = (f32)s_settings->m_shadowDim;
		viewPort.Width = (f32)s_settings->m_shadowDim;
		viewPort.MaxDepth = 1.0f;
		viewPort.MinDepth = 0.0f;
		viewPort.TopLeftX = 0;
		viewPort.TopLeftY = 0;

		D3D12_RECT rectScissor;
		rectScissor.left = 0;
		rectScissor.top = 0;
		rectScissor.right = s_settings->m_shadowDim;
		rectScissor.bottom = s_settings->m_shadowDim;

		m_commandList->Reset(m_cmdAlloc->get(), m_pipelineState.get());

		auto voxelTextureColorTmp = s_resourceManager->getTexture(L"voxelTextureColorTmp");
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(voxelTextureColorTmp->get()));
		auto gpuHandle = m_heapUav.getHandleGpu();
		auto cpuHandle = m_heapUav.getHandleCpu();
		m_commandList->ClearUnorderedAccessViewUint(gpuHandle, cpuHandle, voxelTextureColorTmp->get(), clearValues, 0, nullptr);

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(voxelTextureColorTmp->get()));

		m_commandList->RSSetViewports(1, &viewPort);
		m_commandList->RSSetScissorRects(1, &rectScissor);

		auto srvHeap = m_heapSrv.get();
		m_commandList->SetDescriptorHeaps(1, &srvHeap);

		m_commandList->SetGraphicsRootSignature(m_rootSignature.get());
		m_commandList->SetGraphicsRootDescriptorTable(0, srvHeap->GetGPUDescriptorHandleForHeapStart());
		m_commandList->SetGraphicsRootDescriptorTable(1, srvHeap->GetGPUDescriptorHandleForHeapStart());
		m_commandList->SetGraphicsRootDescriptorTable(2, srvHeap->GetGPUDescriptorHandleForHeapStart());

		D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[2];
		vertexBufferViews[0] = s_resourceManager->getResourceView("meshVertexBufferView")->vbv;
		vertexBufferViews[1] = s_resourceManager->getResourceView("meshDrawIdBufferView")->vbv;
		auto indexBufferView = s_resourceManager->getResourceView("meshIndexBufferView")->ibv;

		auto shadowCastingLightsBuffer = s_resourceManager->getBuffer(L"shadowCastingLightsBuffer");

		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_commandList->IASetVertexBuffers(0, 2, vertexBufferViews);
		m_commandList->IASetIndexBuffer(&indexBufferView);

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowCastingLightsBuffer->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

		auto handleRtv = m_heapRtv.getHandleCpu();
		auto handleDsv = m_heapDsv.getHandleCpu();

		for (u32 i = 0; i < m_visibleLightCount; ++i)
		{
			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = handleDsv;
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[3];
			rtvHandles[0] = handleRtv;

			handleRtv.offset(1);
			rtvHandles[1] = handleRtv;

			handleRtv.offset(1);
			rtvHandles[2] = handleRtv;

			m_commandList->OMSetRenderTargets(3, rtvHandles, FALSE, &dsvHandle);
			m_commandList->ClearRenderTargetView(rtvHandles[0], clearcolor, 0, nullptr);
			m_commandList->ClearRenderTargetView(rtvHandles[1], clearcolor0, 0, nullptr);
			m_commandList->ClearRenderTargetView(rtvHandles[2], clearcolor0, 0, nullptr);
			m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

			m_commandList->SetGraphicsRoot32BitConstant(3, i, 0);

			m_drawCmd->draw(m_commandList.get());

			handleRtv.offset(1);
			handleDsv.offset(1);
		}

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(voxelTextureColorTmp->get()));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowCastingLightsBuffer->get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		auto hr = m_commandList->Close();
		m_buildList = 1;
	}
}

void RenderPassShadow::submitCommands(Graphics::CommandQueue* queue)
{
	if (m_buildList != 0)
	{
		queue->pushCommandList(&m_commandList);
		m_buildList = 0;
	}
}