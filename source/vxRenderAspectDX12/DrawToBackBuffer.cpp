#include "DrawToBackBuffer.h"
#include "d3dx12.h"
#include "Device.h"
#include "CommandQueue.h"
#include "ResourceManager.h"
#include <vxLib/math/Vector.h>
#include "UploadManager.h"
#include "ShaderManager.h"

DrawToBackBuffer::DrawToBackBuffer()
{

}

DrawToBackBuffer::~DrawToBackBuffer()
{

}

bool DrawToBackBuffer::createRootSignature(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangePS[1];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0, 0);

	CD3DX12_ROOT_PARAMETER rootParameters[1];
	rootParameters[0].InitAsDescriptorTable(1, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);

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
	sampler.MaxLOD = 1.0f;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(1, rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	return m_rootSignature.create(&rootSignatureDesc, device);
}

bool DrawToBackBuffer::createPipelineState(ID3D12Device* device, d3d::ShaderManager* shaderManager)
{
	if (!shaderManager->loadShader(L"BackBufferPS.cso"))
		return false;

	auto rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	d3d::PipelineStateDescInput inputDesc;
	inputDesc.rootSignature = m_rootSignature.get();
	inputDesc.depthEnabled = 0;
	inputDesc.shaderDesc.vs = shaderManager->getShader(L"DrawQuadVs.cso");
	inputDesc.shaderDesc.gs = shaderManager->getShader(L"DrawQuadGs.cso");
	inputDesc.shaderDesc.ps = shaderManager->getShader(L"BackBufferPS.cso");
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	inputDesc.rtvFormats = &rtvFormat;
	inputDesc.rtvCount = 1;
	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);

	return d3d::PipelineState::create(desc, &m_pipelineState, device);
}

bool DrawToBackBuffer::initialize(d3d::Device* device, d3d::ShaderManager* shaderManager, d3d::ResourceManager* resourceManager, u32 backBufferCount, u32 frameCount)
{
	if (!createRootSignature(device->getDevice()))
		return false;

	if (!createPipelineState(device->getDevice(), shaderManager))
		return false;

	m_allocators = std::make_unique<d3d::CommandAllocator[]>(frameCount);
	m_commandLists = std::make_unique<d3d::GraphicsCommandList[]>(frameCount);
	for (u32 i = 0; i < frameCount; ++i)
	{
		if (!m_allocators[i].create(D3D12_COMMAND_LIST_TYPE_DIRECT, device->getDevice()))
			return false;

		if (!m_commandLists[i].create(device->getDevice(), D3D12_COMMAND_LIST_TYPE_DIRECT, m_allocators[i].get(), m_pipelineState.get()))
			return false;
	}

	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 1;
	desc.NumDescriptors = backBufferCount;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	if (!m_rtvHeap.create(desc, device->getDevice()))
		return false;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	auto handle = m_rtvHeap.getHandleCpu();
	for (u32 i = 0; i < backBufferCount; ++i)
	{
		ID3D12Resource* rtv = nullptr;
		device->getBuffer(i, &rtv);

		device->getDevice()->CreateRenderTargetView(rtv, &rtvDesc, handle);
		handle.offset(1);
	}

	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NumDescriptors = frameCount * 2;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (!m_srvHeap.create(desc, device->getDevice()))
		return false;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.ArraySize = 1;
	srvDesc.Texture2DArray.MipLevels = 1;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.PlaneSlice = 0;
	srvDesc.Texture2DArray.ResourceMinLODClamp = 0;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	auto layerGameTexture = resourceManager->getTextureRtDs(L"layerGameTexture");
	auto layerOverlayTexture = resourceManager->getTextureRtDs(L"layerOverlayTexture");

	auto srvHandle = m_srvHeap.getHandleCpu();
	for (u32 i = 0; i < frameCount; ++i)
	{
		srvDesc.Texture2DArray.FirstArraySlice = i;

		srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		device->getDevice()->CreateShaderResourceView(layerGameTexture->get(), &srvDesc, srvHandle);
		srvHandle.offset(1);

		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		device->getDevice()->CreateShaderResourceView(layerOverlayTexture->get(), &srvDesc, srvHandle);
		srvHandle.offset(1);
	}

	m_layerGameTexture = layerGameTexture;
	m_layerOverlayTexture = layerOverlayTexture;

	return true;
}

void DrawToBackBuffer::shutdown()
{

}

void DrawToBackBuffer::submitList(d3d::CommandQueue* queue, d3d::Device* device, u32 frameIndex)
{
	const f32 clearColor[4] = { 0.2f, 0.5f, 0.2f, 0 };
	auto currentBackBufferIndex = device->getCurrentBackBufferIndex();
	vx::uint2 resolution(1920, 1080);

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

	ID3D12Resource* rtv = nullptr;
	device->getBuffer(currentBackBufferIndex, &rtv);

	auto &allocator = m_allocators[frameIndex];
	auto &commandList = m_commandLists[frameIndex];
	allocator->Reset();
	commandList->Reset(allocator.get(), m_pipelineState.get());

	commandList->RSSetScissorRects(1, &rectScissor);
	commandList->RSSetViewports(1, &viewport);

	D3D12_RESOURCE_BARRIER barriersStart [] =
	{
		CD3DX12_RESOURCE_BARRIER::Transition(m_layerGameTexture->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, frameIndex),
		CD3DX12_RESOURCE_BARRIER::Transition(m_layerOverlayTexture->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, frameIndex),
		CD3DX12_RESOURCE_BARRIER::Transition(rtv, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET)
	};

	commandList->ResourceBarrier(_countof(barriersStart), barriersStart);

	auto rtvCpuHandle = m_rtvHeap.getHandleCpu();
	rtvCpuHandle.offset(currentBackBufferIndex);

	auto srvDescHeap = m_srvHeap.get();
	commandList->SetDescriptorHeaps(1, &srvDescHeap);
	commandList->SetGraphicsRootSignature(m_rootSignature.get());

	auto gpuHandle = m_srvHeap.getHandleGpu();
	gpuHandle.offset(frameIndex * 2);
	commandList->SetGraphicsRootDescriptorTable(0, gpuHandle);
	//m_commandList->SetGraphicsRootShaderResourceView(0, );

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = rtvCpuHandle;
	commandList->OMSetRenderTargets(1, &cpuHandle, 0, 0);
	commandList->ClearRenderTargetView(cpuHandle, clearColor, 0, 0);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	commandList->DrawInstanced(1, 1, 0, 0);

	D3D12_RESOURCE_BARRIER barriersEnd[] =
	{
		CD3DX12_RESOURCE_BARRIER::Transition(m_layerGameTexture->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, frameIndex),
		CD3DX12_RESOURCE_BARRIER::Transition(m_layerOverlayTexture->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, frameIndex),
		CD3DX12_RESOURCE_BARRIER::Transition(rtv, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)
	};

	commandList->ResourceBarrier(_countof(barriersEnd), barriersEnd);

	commandList->Close();

	queue->pushCommandList(&commandList);
}