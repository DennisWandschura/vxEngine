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
#include "GpuShadowTransform.h"
#include "GpuVoxel.h"
#include "CommandList.h"
#include "GpuProfiler.h"

RenderPassFilterRSM::RenderPassFilterRSM()
	:RenderPassLight(),
	m_buildList(0)
{

}

RenderPassFilterRSM::~RenderPassFilterRSM()
{

}

void RenderPassFilterRSM::getRequiredMemory(u64* heapSizeBuffer, u32* bufferCount, u64* heapSizeTexture, u32* textureCount, u64* heapSizeRtDs, u32* rtDsCount, ID3D12Device* device)
{
	const u32 resolution0 = s_settings->m_shadowDim;

	D3D12_RESOURCE_DESC resDesc;
	resDesc.Alignment = 64 KBYTE;
	resDesc.DepthOrArraySize = s_settings->m_shadowCastingLightCount * 6;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Height = resolution0;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.MipLevels = 2;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Width = resolution0;

	auto allocColor0 = device->GetResourceAllocationInfo(1, 1, &resDesc);

	resDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	auto allocNormal0 = device->GetResourceAllocationInfo(1, 1, &resDesc);

	resDesc.Format = DXGI_FORMAT_R32_FLOAT;
	auto allocDepth = device->GetResourceAllocationInfo(1, 1, &resDesc);

	*heapSizeRtDs += allocColor0.SizeInBytes + allocNormal0.SizeInBytes + allocDepth.SizeInBytes;
	*rtDsCount += 3;
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
	resDesc.MipLevels = 2;
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

	clearValues[0].Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	resDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	alloc = device->GetResourceAllocationInfo(1, 1, &resDesc);
	desc.size = alloc.SizeInBytes;

	ptr = s_resourceManager->createTextureRtDs(L"rsmFilteredNormal", desc);
	if (ptr == nullptr)
		return false;

	clearValues[0].Format = DXGI_FORMAT_R32_FLOAT;
	clearValues[0].Color[0] = 1.0f;
	resDesc.Format = DXGI_FORMAT_R32_FLOAT;
	alloc = device->GetResourceAllocationInfo(1, 1, &resDesc);
	desc.size = alloc.SizeInBytes;
	ptr = s_resourceManager->createTextureRtDs(L"rsmFilteredDepth", desc);
	if (ptr == nullptr)
		return false;

	return true;
}

bool RenderPassFilterRSM::loadShaders()
{
	if (!s_shaderManager->loadShader(L"ShadowMapFilterVS.cso"))
		return false;

	if (!s_shaderManager->loadShader(L"ShadowMapFilterGS.cso"))
		return false;

	if (!s_shaderManager->loadShader(L"ShadowMapFilterPS.cso"))
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

	return m_rootSignature.create(&rootSignatureDesc, device);
}

bool RenderPassFilterRSM::createPipelineState(ID3D12Device* device)
{
	DXGI_FORMAT formats[3] =
	{
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		DXGI_FORMAT_R32_FLOAT
	};

	d3d::PipelineStateDescInput inputDesc;
	inputDesc.inputLayout.pInputElementDescs = nullptr;
	inputDesc.inputLayout.NumElements = 0;
	inputDesc.depthEnabled = 0;
	inputDesc.rootSignature = m_rootSignature.get();
	inputDesc.shaderDesc.vs = s_shaderManager->getShader(L"ShadowMapFilterVS.cso");
	inputDesc.shaderDesc.gs = s_shaderManager->getShader(L"ShadowMapFilterGS.cso");
	inputDesc.shaderDesc.ps = s_shaderManager->getShader(L"ShadowMapFilterPS.cso");
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	inputDesc.rtvCount = 3;
	inputDesc.rtvFormats = formats;

	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);

	return d3d::PipelineState::create(desc, &m_pipelineState, device);
}

bool RenderPassFilterRSM::createSrvHeap(ID3D12Device* device)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 5;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	if (!m_srvHeap.create(desc, device))
		return false;

	if (!m_srvHeapPass2.create(desc, device))
		return false;

	return true;
}

bool RenderPassFilterRSM::createRtvHeap(ID3D12Device* device)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 6;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	if (!m_rtvHeap.create(desc, device))
		return false;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Texture2DArray.ArraySize = 10 * 6;
	rtvDesc.Texture2DArray.FirstArraySlice = 0;
	rtvDesc.Texture2DArray.MipSlice = 0;
	rtvDesc.Texture2DArray.PlaneSlice = 0;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;

	auto handle = m_rtvHeap.getHandleCpu();

	auto rsmFilteredColor = s_resourceManager->getTextureRtDs(L"rsmFilteredColor");
	auto rsmFilteredNormal = s_resourceManager->getTextureRtDs(L"rsmFilteredNormal");
	auto rsmFilteredDepth = s_resourceManager->getTextureRtDs(L"rsmFilteredDepth");
	{
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		device->CreateRenderTargetView(rsmFilteredColor->get(), &rtvDesc, handle);

		rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		handle.offset(1);
		device->CreateRenderTargetView(rsmFilteredNormal->get(), &rtvDesc, handle);

		rtvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		handle.offset(1);
		device->CreateRenderTargetView(rsmFilteredDepth->get(), &rtvDesc, handle);
	}
	{
		rtvDesc.Texture2DArray.MipSlice = 1;

		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		handle.offset(1);
		device->CreateRenderTargetView(rsmFilteredColor->get(), &rtvDesc, handle);

		rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		handle.offset(1);
		device->CreateRenderTargetView(rsmFilteredNormal->get(), &rtvDesc, handle);

		rtvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		handle.offset(1);
		device->CreateRenderTargetView(rsmFilteredDepth->get(), &rtvDesc, handle);
	}

	return true;
}

void RenderPassFilterRSM::createViews(ID3D12Device* device)
{
	/*

Texture2DArray g_srcRSMColor : register(t0);
Texture2DArray<half2> g_srcRSMNormal : register(t1);
Texture2DArray<float> g_srcRSMDepth : register(t2);
StructuredBuffer<GpuShadowTransformReverse> g_transforms : register(t3);

cbuffer lpvConstantBuffer : register(b0)
*/

	auto shadowTextureColor = s_resourceManager->getTextureRtDs(L"shadowTextureColor");
	auto shadowTextureNormal = s_resourceManager->getTextureRtDs(L"shadowTextureNormal");
	auto shadowTextureDepth = s_resourceManager->getTextureRtDs(L"shadowTextureDepth");
	auto shadowReverseTransformBuffer = s_resourceManager->getBuffer(L"shadowReverseTransformBuffer");
	auto voxelBuffer = s_resourceManager->getBuffer(L"voxelBuffer");

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescRsmColor;
	srvDescRsmColor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDescRsmColor.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescRsmColor.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDescRsmColor.Texture2DArray.ArraySize = s_settings->m_shadowCastingLightCount * 6;
	srvDescRsmColor.Texture2DArray.FirstArraySlice = 0;
	srvDescRsmColor.Texture2DArray.MipLevels = 1;
	srvDescRsmColor.Texture2DArray.MostDetailedMip = 0;
	srvDescRsmColor.Texture2DArray.PlaneSlice = 0;
	srvDescRsmColor.Texture2DArray.ResourceMinLODClamp = 0;

	auto handle = m_srvHeap.getHandleCpu();
	device->CreateShaderResourceView(shadowTextureColor->get(), &srvDescRsmColor, handle);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescRsmNormals;
	srvDescRsmNormals.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvDescRsmNormals.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescRsmNormals.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDescRsmNormals.Texture2DArray.ArraySize = s_settings->m_shadowCastingLightCount * 6;
	srvDescRsmNormals.Texture2DArray.FirstArraySlice = 0;
	srvDescRsmNormals.Texture2DArray.MipLevels = 1;
	srvDescRsmNormals.Texture2DArray.MostDetailedMip = 0;
	srvDescRsmNormals.Texture2DArray.PlaneSlice = 0;
	srvDescRsmNormals.Texture2DArray.ResourceMinLODClamp = 0;

	handle.offset(1);
	device->CreateShaderResourceView(shadowTextureNormal->get(), &srvDescRsmNormals, handle);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescRsmDepth;
	srvDescRsmDepth.Format = DXGI_FORMAT_R32_FLOAT;
	srvDescRsmDepth.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescRsmDepth.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDescRsmDepth.Texture2DArray.ArraySize = s_settings->m_shadowCastingLightCount * 6;
	srvDescRsmDepth.Texture2DArray.FirstArraySlice = 0;
	srvDescRsmDepth.Texture2DArray.MipLevels = 1;
	srvDescRsmDepth.Texture2DArray.MostDetailedMip = 0;
	srvDescRsmDepth.Texture2DArray.PlaneSlice = 0;
	srvDescRsmDepth.Texture2DArray.ResourceMinLODClamp = 0;

	handle.offset(1);
	device->CreateShaderResourceView(shadowTextureDepth->get(), &srvDescRsmDepth, handle);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescBuffer;
	srvDescBuffer.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDescBuffer.Format = DXGI_FORMAT_UNKNOWN;
	srvDescBuffer.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescBuffer.Buffer.FirstElement = 0;
	srvDescBuffer.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDescBuffer.Buffer.NumElements = s_settings->m_shadowCastingLightCount;
	srvDescBuffer.Buffer.StructureByteStride = sizeof(GpuShadowTransformReverse);

	handle.offset(1);
	device->CreateShaderResourceView(shadowReverseTransformBuffer->get(), &srvDescBuffer, handle);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = voxelBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = d3d::AlignedSizeType<GpuVoxel, 1, 256>::size;;

	handle.offset(1);
	device->CreateConstantBufferView(&cbvDesc, handle);
}

void RenderPassFilterRSM::createViewsPass2(ID3D12Device* device)
{
	auto rsmFilteredColor = s_resourceManager->getTextureRtDs(L"rsmFilteredColor");
	auto rsmFilteredNormal = s_resourceManager->getTextureRtDs(L"rsmFilteredNormal");
	auto rsmFilteredDepth = s_resourceManager->getTextureRtDs(L"rsmFilteredDepth");
	auto shadowReverseTransformBuffer = s_resourceManager->getBuffer(L"shadowReverseTransformBuffer");
	auto voxelBuffer = s_resourceManager->getBuffer(L"voxelBuffer");

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescRsmColor;
	srvDescRsmColor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDescRsmColor.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescRsmColor.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDescRsmColor.Texture2DArray.ArraySize = s_settings->m_shadowCastingLightCount * 6;
	srvDescRsmColor.Texture2DArray.FirstArraySlice = 0;
	srvDescRsmColor.Texture2DArray.MipLevels = 1;
	srvDescRsmColor.Texture2DArray.MostDetailedMip = 0;
	srvDescRsmColor.Texture2DArray.PlaneSlice = 0;
	srvDescRsmColor.Texture2DArray.ResourceMinLODClamp = 0;

	auto handle = m_srvHeapPass2.getHandleCpu();
	device->CreateShaderResourceView(rsmFilteredColor->get(), &srvDescRsmColor, handle);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescRsmNormals;
	srvDescRsmNormals.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvDescRsmNormals.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescRsmNormals.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDescRsmNormals.Texture2DArray.ArraySize = s_settings->m_shadowCastingLightCount * 6;
	srvDescRsmNormals.Texture2DArray.FirstArraySlice = 0;
	srvDescRsmNormals.Texture2DArray.MipLevels = 1;
	srvDescRsmNormals.Texture2DArray.MostDetailedMip = 0;
	srvDescRsmNormals.Texture2DArray.PlaneSlice = 0;
	srvDescRsmNormals.Texture2DArray.ResourceMinLODClamp = 0;

	handle.offset(1);
	device->CreateShaderResourceView(rsmFilteredNormal->get(), &srvDescRsmNormals, handle);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescRsmDepth;
	srvDescRsmDepth.Format = DXGI_FORMAT_R32_FLOAT;
	srvDescRsmDepth.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescRsmDepth.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDescRsmDepth.Texture2DArray.ArraySize = s_settings->m_shadowCastingLightCount * 6;
	srvDescRsmDepth.Texture2DArray.FirstArraySlice = 0;
	srvDescRsmDepth.Texture2DArray.MipLevels = 1;
	srvDescRsmDepth.Texture2DArray.MostDetailedMip = 0;
	srvDescRsmDepth.Texture2DArray.PlaneSlice = 0;
	srvDescRsmDepth.Texture2DArray.ResourceMinLODClamp = 0;

	handle.offset(1);
	device->CreateShaderResourceView(rsmFilteredDepth->get(), &srvDescRsmDepth, handle);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescBuffer;
	srvDescBuffer.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDescBuffer.Format = DXGI_FORMAT_UNKNOWN;
	srvDescBuffer.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescBuffer.Buffer.FirstElement = 0;
	srvDescBuffer.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDescBuffer.Buffer.NumElements = s_settings->m_shadowCastingLightCount;
	srvDescBuffer.Buffer.StructureByteStride = sizeof(GpuShadowTransformReverse);

	handle.offset(1);
	device->CreateShaderResourceView(shadowReverseTransformBuffer->get(), &srvDescBuffer, handle);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = voxelBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = d3d::AlignedSizeType<GpuVoxel, 1, 256>::size;;

	handle.offset(1);
	device->CreateConstantBufferView(&cbvDesc, handle);
}

bool RenderPassFilterRSM::initialize(ID3D12Device* device, d3d::CommandAllocator* allocators, u32 frameCount)
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

	if (!createCommandLists(device, D3D12_COMMAND_LIST_TYPE_DIRECT, allocators, frameCount))
	{
		return false;
	}

	createViews(device);
	createViewsPass2(device);

	return true;
}

void RenderPassFilterRSM::shutdown()
{

}

void RenderPassFilterRSM::buildCommands(d3d::CommandAllocator* currentAllocator, u32 frameIndex)
{
	const u32 resolution = s_settings->m_shadowDim / 2;
	const f32 clearColor[4] = { 0, 0, 0, 0 };
	const f32 clearColor1[4] = { 1, 0, 0, 0 };

	if (m_visibleLightCount != 0)
	{
		auto &commandList = m_commandLists[frameIndex];
		m_currentCommandList = &commandList;

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

		commandList->Reset(currentAllocator->get(), m_pipelineState.get());

		s_gpuProfiler->queryBegin("filter rsm", &commandList);

		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &rectScissor);

		auto rtvHandle = m_rtvHeap.getHandleCpu();

		auto shadowTextureColor = s_resourceManager->getTextureRtDs(L"shadowTextureColor");
		auto shadowTextureNormal = s_resourceManager->getTextureRtDs(L"shadowTextureNormal");
		auto shadowTextureDepth = s_resourceManager->getTextureRtDs(L"shadowTextureDepth");

		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowTextureColor->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0));
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowTextureNormal->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0));
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowTextureDepth->get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0));

		D3D12_CPU_DESCRIPTOR_HANDLE handles[3];
		handles[0] = rtvHandle;
		rtvHandle.offset(1);
		handles[1] = rtvHandle;
		rtvHandle.offset(1);
		handles[2] = rtvHandle;

		commandList->OMSetRenderTargets(3, handles, 0, nullptr);

		auto srvHeap = m_srvHeap.get();
		commandList->SetDescriptorHeaps(1, &srvHeap);
		commandList->SetGraphicsRootSignature(m_rootSignature.get());
		commandList->SetGraphicsRootDescriptorTable(0, srvHeap->GetGPUDescriptorHandleForHeapStart());

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		commandList->DrawInstanced(m_visibleLightCount, 1, 0, 0);

		CD3DX12_RESOURCE_BARRIER barrier0[] =
		{
			CD3DX12_RESOURCE_BARRIER::Transition(shadowTextureColor->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0),
			CD3DX12_RESOURCE_BARRIER::Transition(shadowTextureNormal->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0),
			CD3DX12_RESOURCE_BARRIER::Transition(shadowTextureDepth->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE, 0)
		};

		commandList->ResourceBarrier(_countof(barrier0), barrier0);

		//////////////////////
		rtvHandle.offset(1);
		handles[0] = rtvHandle;
		rtvHandle.offset(1);
		handles[1] = rtvHandle;
		rtvHandle.offset(1);
		handles[2] = rtvHandle;

		viewport.Height = (f32)resolution / 2;
		viewport.Width = (f32)resolution / 2;
		rectScissor.right = resolution / 2;
		rectScissor.bottom = resolution / 2;

		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &rectScissor);

		auto rsmFilteredColor = s_resourceManager->getTextureRtDs(L"rsmFilteredColor");
		auto rsmFilteredNormal = s_resourceManager->getTextureRtDs(L"rsmFilteredNormal");
		auto rsmFilteredDepth = s_resourceManager->getTextureRtDs(L"rsmFilteredDepth");

		CD3DX12_RESOURCE_BARRIER barrier1[] =
		{
			CD3DX12_RESOURCE_BARRIER::Transition(rsmFilteredColor->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0),
			CD3DX12_RESOURCE_BARRIER::Transition(rsmFilteredNormal->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0),
			CD3DX12_RESOURCE_BARRIER::Transition(rsmFilteredDepth->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0)
		};

		commandList->ResourceBarrier(_countof(barrier1), barrier1);

		commandList->OMSetRenderTargets(3, handles, 0, nullptr);

		srvHeap = m_srvHeapPass2.get();
		commandList->SetDescriptorHeaps(1, &srvHeap);
		commandList->SetGraphicsRootDescriptorTable(0, srvHeap->GetGPUDescriptorHandleForHeapStart());

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		commandList->DrawInstanced(m_visibleLightCount, 6, 0, 0);

		CD3DX12_RESOURCE_BARRIER barrier2[] =
		{
			CD3DX12_RESOURCE_BARRIER::Transition(rsmFilteredColor->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0),
			CD3DX12_RESOURCE_BARRIER::Transition(rsmFilteredNormal->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0),
			CD3DX12_RESOURCE_BARRIER::Transition(rsmFilteredDepth->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0)
		};

		commandList->ResourceBarrier(_countof(barrier2), barrier2);
		////////////////

		s_gpuProfiler->queryEnd(&commandList);

		commandList->Close();
		m_buildList = 1;
	}
}

void RenderPassFilterRSM::submitCommands(Graphics::CommandQueue* queue)
{
	if (m_buildList != 0)
	{
		queue->pushCommandList(m_currentCommandList);
		m_buildList = 0;
	}
}