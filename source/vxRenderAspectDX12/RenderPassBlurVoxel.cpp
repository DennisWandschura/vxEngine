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

#include "RenderPassBlurVoxel.h"
#include "CommandAllocator.h"
#include "ShaderManager.h"
#include "ResourceManager.h"
#include "ResourceDesc.h"
#include "ShaderResourceViewDesc.h"

RenderPassBlurVoxel::RenderPassBlurVoxel(d3d::CommandAllocator* cmdAlloc)
	:m_commandList(),
	m_cmdAlloc(cmdAlloc),
	m_srvHeap(),
	m_rtvHeap()
{

}

RenderPassBlurVoxel::~RenderPassBlurVoxel()
{

}

void RenderPassBlurVoxel::getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device)
{
	auto blurTargetDesc = d3d::ResourceDesc::getDescTexture2D(s_resolution, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
	auto blurAllocInfo = device->GetResourceAllocationInfo(1, 1,&blurTargetDesc);

	*heapSizeRtDs += blurAllocInfo.SizeInBytes * 2;
}

bool RenderPassBlurVoxel::createData(ID3D12Device* device)
{
	auto blurTargetDesc = d3d::ResourceDesc::getDescTexture2D(s_resolution, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
	auto blurAllocInfo = device->GetResourceAllocationInfo(1, 1, &blurTargetDesc);

	D3D12_CLEAR_VALUE clearValue;
	memset(&clearValue, 0, sizeof(clearValue));
	clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	CreateResourceDesc desc = CreateResourceDesc::createDesc(blurAllocInfo.SizeInBytes, &blurTargetDesc, &clearValue, D3D12_RESOURCE_STATE_RENDER_TARGET);
	auto ptr = s_resourceManager->createTextureRtDs(L"voxelBlur", desc);
	if (ptr == nullptr)
		return false;

	ptr = s_resourceManager->createTextureRtDs(L"voxelBlurTmp", desc);
	if (ptr == nullptr)
		return false;

	return true;
}

bool RenderPassBlurVoxel::loadShaders()
{
	if (!s_shaderManager->loadShader(L"VoxelBlurPS.cso"))
		return false;

	if (!s_shaderManager->loadShader(L"VoxelBlurPass2PS.cso"))
		return false;

	return true;
}

bool RenderPassBlurVoxel::createRootSignature(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangePS[1];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, 0);

	CD3DX12_ROOT_PARAMETER rootParameters[2];
	rootParameters[0].InitAsDescriptorTable(1, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[1].InitAsConstants(1, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = 0.0f;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(2, rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	return m_rootSignature.create(&rootSignatureDesc, device);
}

bool RenderPassBlurVoxel::createPipelineState(ID3D12Device* device)
{
	auto rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3d::PipelineStateDescInput inputDesc;
	inputDesc.shaderDesc.vs = s_shaderManager->getShader(L"DrawQuadVs.cso");
	inputDesc.shaderDesc.gs = s_shaderManager->getShader(L"DrawQuadGs.cso");
	inputDesc.shaderDesc.ps = s_shaderManager->getShader(L"VoxelBlurPS.cso");
	inputDesc.rtvCount = 1;
	inputDesc.depthEnabled = 0;
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	inputDesc.rtvFormats = &rtvFormat;
	inputDesc.rootSignature = m_rootSignature.get();

	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);

	return d3d::PipelineState::create(desc, &m_pipelineState, device);
}

bool RenderPassBlurVoxel::createPipelineStateBlur2(ID3D12Device* device)
{
	auto rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3d::PipelineStateDescInput inputDesc;
	inputDesc.shaderDesc.vs = s_shaderManager->getShader(L"DrawQuadVs.cso");
	inputDesc.shaderDesc.gs = s_shaderManager->getShader(L"DrawQuadGs.cso");
	inputDesc.shaderDesc.ps = s_shaderManager->getShader(L"VoxelBlurPass2PS.cso");
	inputDesc.rtvCount = 1;
	inputDesc.depthEnabled = 0;
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	inputDesc.rtvFormats = &rtvFormat;
	inputDesc.rootSignature = m_rootSignature.get();

	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);

	return d3d::PipelineState::create(desc, &m_pipelineStateBlur2, device);
}

void RenderPassBlurVoxel::createViews(ID3D12Device* device)
{

}

bool RenderPassBlurVoxel::initialize(ID3D12Device* device, void* p)
{
	if (!loadShaders())
		return false;

	if (!createRootSignature(device))
		return false;

	if (!createPipelineState(device))
		return false;

	if (!createPipelineStateBlur2(device))
		return false;

	if (!m_commandList.create(device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAlloc->get(), m_pipelineState.get()))
		return false;

	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 3;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (!m_srvHeap.create(desc, device))
		return false;

	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.NumDescriptors = 2;
	if (!m_rtvHeap.create(desc, device))
		return false;

	d3d::ShaderResourceViewDesc srvDesc = d3d::ShaderResourceViewDesc::getDescTexture2D(DXGI_FORMAT_R8G8B8A8_UNORM);
	auto voxelIndirect = s_resourceManager->getTextureRtDs(L"voxelIndirect");
	auto srvHandle = m_srvHeap.getHandleCpu();
	device->CreateShaderResourceView(voxelIndirect->get(), &srvDesc, srvHandle);

	srvHandle.offset(1);
	auto voxelBlurTmp = s_resourceManager->getTextureRtDs(L"voxelBlurTmp");
	device->CreateShaderResourceView(voxelBlurTmp->get(), &srvDesc, srvHandle);

	srvHandle.offset(1);
	auto voxelBlur = s_resourceManager->getTextureRtDs(L"voxelBlur");
	device->CreateShaderResourceView(voxelBlur->get(), &srvDesc, srvHandle);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	auto rtvHandle = m_rtvHeap.getHandleCpu();
	device->CreateRenderTargetView(voxelBlur->get(), &rtvDesc, rtvHandle);

	rtvHandle.offset(1);
	device->CreateRenderTargetView(voxelBlurTmp->get(), &rtvDesc, rtvHandle);

	return true;
}

void RenderPassBlurVoxel::shutdown()
{

}

void RenderPassBlurVoxel::blurPass(ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE* handleRtv, const D3D12_GPU_DESCRIPTOR_HANDLE &handleSrv, u32 pixelDist)
{
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	m_commandList->OMSetRenderTargets(1, handleRtv, 0, nullptr);

	m_commandList->SetGraphicsRootDescriptorTable(0, handleSrv);

	m_commandList->SetGraphicsRoot32BitConstant(1, pixelDist, 0);

	m_commandList->DrawInstanced(1, 1, 0, 0);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
}

void RenderPassBlurVoxel::submitCommands(Graphics::CommandQueue* queue)
{
	auto voxelIndirect = s_resourceManager->getTextureRtDs(L"voxelIndirect");
	auto voxelBlurTmp = s_resourceManager->getTextureRtDs(L"voxelBlurTmp");
	auto voxelBlur = s_resourceManager->getTextureRtDs(L"voxelBlur");

	const f32 clearColor[4] = {0, 0, 0, 0};
	m_commandList->Reset(m_cmdAlloc->get(), m_pipelineState.get());

	auto resolution = s_resolution;

	D3D12_VIEWPORT viewport;
	viewport.Height = (f32)resolution.y;
	viewport.Width = (f32)resolution.x;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	D3D12_RECT rectScissor;
	rectScissor.left = 0;
	rectScissor.top = 0;
	rectScissor.right = resolution.x;
	rectScissor.bottom = resolution.y;

	m_commandList->RSSetViewports(1, &viewport);
	m_commandList->RSSetScissorRects(1, &rectScissor);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(voxelIndirect->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	auto rtvHandle = m_rtvHeap.getHandleCpu();

	D3D12_CPU_DESCRIPTOR_HANDLE handleRt = rtvHandle;
	rtvHandle.offset(1);
	D3D12_CPU_DESCRIPTOR_HANDLE handleRtTmp = rtvHandle;

	m_commandList->OMSetRenderTargets(1, &handleRtTmp, 0, nullptr);
	//m_commandList->ClearRenderTargetView(handleRtTmp, clearColor, 0, nullptr);

	auto srvHeap = m_srvHeap.get();
	m_commandList->SetDescriptorHeaps(1, &srvHeap);
	m_commandList->SetGraphicsRootSignature(m_rootSignature.get());

	auto gpuHandle = m_srvHeap.getHandleGpu();
	m_commandList->SetGraphicsRootDescriptorTable(0, gpuHandle);

	gpuHandle.offset(1);
	auto gpuHandleSrvBlurTmp = gpuHandle;

	gpuHandle.offset(1);
	auto gpuHandleSrvBlur = gpuHandle;

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	m_commandList->DrawInstanced(1, 1, 0, 0);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(voxelIndirect->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));

	m_commandList->SetPipelineState(m_pipelineStateBlur2.get());
	blurPass(voxelBlurTmp->get(), &handleRt, gpuHandleSrvBlurTmp, 1);
	/*blurPass(voxelBlur->get(), &handleRtTmp, gpuHandleSrvBlur, 2);
	blurPass(voxelBlurTmp->get(), &handleRt, gpuHandleSrvBlurTmp, 4);
	blurPass(voxelBlur->get(), &handleRtTmp, gpuHandleSrvBlur, 8);
	blurPass(voxelBlurTmp->get(), &handleRt, gpuHandleSrvBlurTmp, 16);
	blurPass(voxelBlur->get(), &handleRtTmp, gpuHandleSrvBlur, 32);
	blurPass(voxelBlurTmp->get(), &handleRt, gpuHandleSrvBlurTmp, 64);*/

	/*m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(voxelBlurTmp->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	m_commandList->OMSetRenderTargets(1, &handleRt, 0, nullptr);

	u32 pixelDist = 2;
	m_commandList->SetGraphicsRootDescriptorTable(0, gpuHandleSrvBlurTmp);
	m_commandList->SetGraphicsRoot32BitConstant(1, pixelDist, 0);

	m_commandList->DrawInstanced(1, 1, 0, 0);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(voxelBlurTmp->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));*/

	m_commandList->Close();
	queue->pushCommandList(&m_commandList);
}