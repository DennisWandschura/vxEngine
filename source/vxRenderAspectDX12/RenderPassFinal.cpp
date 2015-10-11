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
#include "RenderPassFinal.h"
#include "ShaderManager.h"
#include "d3dx12.h"
#include "ResourceManager.h"
#include "Device.h"
#include "CommandAllocator.h"
#include "GpuProfiler.h"
#include "FrameData.h"
#include "ResourceDesc.h"

RenderPassFinal::RenderPassFinal(d3d::Device* device)
	:RenderPass(),
	m_descriptorHeapSrv(),
	m_descriptorHeapRtv(),
	m_device(device)
{

}

RenderPassFinal::~RenderPassFinal()
{

}

void RenderPassFinal::getRequiredMemory(u64* heapSizeBuffer, u32* bufferCount, u64* heapSizeTexture, u32* textureCount, u64* heapSizeRtDs, u32* rtDsCount, ID3D12Device* device)
{
	d3d::ResourceDesc resDesc = d3d::ResourceDesc::getDescTexture2D(s_resolution, DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
	resDesc.DepthOrArraySize = 2;
	auto allocInfo = device->GetResourceAllocationInfo(1, 1, &resDesc);

	*heapSizeRtDs += allocInfo.SizeInBytes;
	*rtDsCount += 1;
}

bool RenderPassFinal::createData(ID3D12Device* device)
{
	d3d::ResourceDesc resDesc = d3d::ResourceDesc::getDescTexture2D(s_resolution, DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
	resDesc.DepthOrArraySize = 2;
	auto allocInfo = device->GetResourceAllocationInfo(1, 1, &resDesc);

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = resDesc.Format;

	CreateResourceDesc desc;
	desc.clearValue = &clearValue;
	desc.resDesc = &resDesc;
	desc.size = allocInfo.SizeInBytes;
	desc.state = D3D12_RESOURCE_STATE_RENDER_TARGET;

	s_resourceManager->createTextureRtDs(L"layerGameTexture", desc);

	return true;
}

bool RenderPassFinal::loadShaders(d3d::ShaderManager* shaderManager)
{
	if (!shaderManager->loadShader(L"DrawQuadVs.cso"))
		return false;

	if (!shaderManager->loadShader(L"DrawQuadGs.cso"))
		return false;

	if (!shaderManager->loadShader(L"FinalPS.cso"))
		return false;

	return true;
}

bool RenderPassFinal::createRootSignature(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangePS[1];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0, 0, 0);

	CD3DX12_ROOT_PARAMETER rootParameters[1];
	rootParameters[0].InitAsDescriptorTable(1, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = 2.0f;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(1, rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	return m_rootSignature.create(&rootSignatureDesc, device);
}

bool RenderPassFinal::createPipelineState(ID3D12Device* device, d3d::ShaderManager* shaderManager)
{
	auto rtvFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

	d3d::PipelineStateDescInput inputDesc;
	inputDesc.rootSignature = m_rootSignature.get();
	inputDesc.depthEnabled = 0;
	inputDesc.shaderDesc.vs = s_shaderManager->getShader(L"DrawQuadVs.cso");
	inputDesc.shaderDesc.gs = s_shaderManager->getShader(L"DrawQuadGs.cso");
	inputDesc.shaderDesc.ps = s_shaderManager->getShader(L"FinalPS.cso");
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	inputDesc.rtvFormats = &rtvFormat;
	inputDesc.rtvCount = 1;
	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);

	return d3d::PipelineState::create(desc, &m_pipelineState, device);
}

bool RenderPassFinal::initialize(ID3D12Device* device, d3d::CommandAllocator* allocators, u32 frameCount)
{
	auto directLightTexture = s_resourceManager->getTextureRtDs(L"directLightTexture");
	auto indirectLightTexture = s_resourceManager->getTextureRtDs(L"indirectLightTexture");
	auto aoTexture = s_resourceManager->getTextureRtDs(L"aoTexture");

	if (!loadShaders(s_shaderManager))
		return false;

	if (!createRootSignature(device))
		return false;

	if (!createPipelineState(device, s_shaderManager))
		return false;

	if (!createCommandLists(device, D3D12_COMMAND_LIST_TYPE_DIRECT, allocators, frameCount))
		return false;

	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 5;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (!m_descriptorHeapSrv.create(desc, device))
		return false;

	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 2;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	if (!m_descriptorHeapRtv.create(desc, device))
		return false;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0;

	auto handle = m_descriptorHeapSrv.getHandleCpu();
	device->CreateShaderResourceView(aoTexture->get(), &srvDesc, handle);

	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	handle.offset(1);
	device->CreateShaderResourceView(directLightTexture->get(), &srvDesc, handle);

	handle.offset(1);
	device->CreateShaderResourceView(indirectLightTexture->get(), &srvDesc, handle);

	auto albedoSlice = s_resourceManager->getTextureRtDs(L"gbufferAlbedo");

	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels=1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0;

	handle.offset(1);
	device->CreateShaderResourceView(albedoSlice->get(), &srvDesc, handle);

	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0;

	auto voxelIndirect = s_resourceManager->getTextureRtDs(L"voxelIndirect");
	handle.offset(1);
	device->CreateShaderResourceView(voxelIndirect->get(), &srvDesc, handle);

	auto rtvHandle = m_descriptorHeapRtv.getHandleCpu();
	auto layerGameTexture = s_resourceManager->getTextureRtDs(L"layerGameTexture");
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	rtvDesc.Texture2DArray.ArraySize = 1;
	rtvDesc.Texture2DArray.MipSlice = 0;
	rtvDesc.Texture2DArray.PlaneSlice = 0;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;

	for (u32 i = 0; i < 2; ++i)
	{
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		device->CreateRenderTargetView(layerGameTexture->get(), &rtvDesc, rtvHandle);

		rtvHandle.offset(1);
	}

	return true;
}

void RenderPassFinal::shutdown()
{
	m_descriptorHeapRtv.destroy();
	m_descriptorHeapSrv.destroy();
}

void RenderPassFinal::buildCommands(d3d::CommandAllocator* currentAllocator, u32 frameIndex)
{
	//auto aoTexture = s_resourceManager->getTextureRtDs(L"aoTexture");
	//auto directLightTexture = s_resourceManager->getTextureRtDs(L"directLightTexture");
	//auto indirectLightTexture = s_resourceManager->getTextureRtDs(L"indirectLightTexture");
	//auto albedoSlice = s_resourceManager->getTextureRtDs(L"gbufferAlbedo");
	//auto voxelIndirect = s_resourceManager->getTextureRtDs(L"voxelIndirect");

	auto &commandList = m_commandLists[frameIndex];
	m_currentCommandList = &commandList;

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

	const f32 clearColor[] = { 0, 0, 0, 0 };
	auto rtvHandle = m_descriptorHeapRtv.getHandleCpu();
	rtvHandle.offset(frameIndex);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[1];
	rtvHandles[0] = rtvHandle;

	auto hresult = commandList->Reset(currentAllocator->get(), m_pipelineState.get());
	VX_ASSERT(hresult == 0);

	s_gpuProfiler->queryBegin("final", &commandList);

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &rectScissor);

	/*m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(aoTexture->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(directLightTexture->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(indirectLightTexture->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(albedoSlice->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(voxelIndirect->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));*/
	//commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(rtv, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	commandList->SetGraphicsRootSignature(m_rootSignature.get());

	ID3D12DescriptorHeap* heaps[] =
	{
		m_descriptorHeapSrv.get()
	};

	commandList->SetDescriptorHeaps(1, heaps);
	commandList->SetGraphicsRootDescriptorTable(0, m_descriptorHeapSrv->GetGPUDescriptorHandleForHeapStart());

	commandList->OMSetRenderTargets(1, rtvHandles, FALSE, nullptr);
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	commandList->DrawInstanced(1, 1, 0, 0);

//	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(rtv, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	/*m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(voxelIndirect->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(albedoSlice->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(indirectLightTexture->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(directLightTexture->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(aoTexture->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));*/

	s_gpuProfiler->queryEnd(&commandList);

	hresult = commandList->Close();
	VX_ASSERT(hresult == 0);
}

void RenderPassFinal::submitCommands(Graphics::CommandQueue* queue)
{
	queue->pushCommandList(m_currentCommandList);
}