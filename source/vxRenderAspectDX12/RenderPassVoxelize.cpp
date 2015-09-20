#include "RenderPassVoxelize.h"
#include "ResourceManager.h"
#include "ShaderManager.h"
#include "GpuVoxel.h"
#include "UploadManager.h"
#include "d3dx12.h"
#include "ResourceView.h"
#include "CommandAllocator.h"

RenderPassVoxelize::RenderPassVoxelize(d3d::CommandAllocator* cmdAlloc, DrawIndexedIndirectCommand* drawCommand)
	:m_cmdAlloc(cmdAlloc),
	m_drawCommand(drawCommand)
{

}

RenderPassVoxelize::~RenderPassVoxelize()
{

}

void RenderPassVoxelize::getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device)
{
	auto resolution = s_settings->m_lpvDim * 4;

	D3D12_RESOURCE_DESC resDesc;
	resDesc.Alignment = 64 KBYTE;
	resDesc.DepthOrArraySize = 1;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Height = resolution;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 4;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Width = resolution;

	auto allocInfo = device->GetResourceAllocationInfo(1, 1, &resDesc);

	*heapSizeRtDs += allocInfo.SizeInBytes;
}

bool RenderPassVoxelize::createData(ID3D12Device* device)
{
	auto resolution = s_settings->m_lpvDim * 4;

	D3D12_RESOURCE_DESC resDesc;
	resDesc.Alignment = 64 KBYTE;
	resDesc.DepthOrArraySize = 1;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Height = resolution;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 4;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Width = resolution;

	auto allocInfo = device->GetResourceAllocationInfo(1, 1, &resDesc);

	D3D12_CLEAR_VALUE clearValue;
	memset(&clearValue, 0, sizeof(clearValue));
	clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	CreateResourceDesc desc;
	desc.clearValue = &clearValue;
	desc.resDesc = &resDesc;
	desc.size = allocInfo.SizeInBytes;
	desc.state = D3D12_RESOURCE_STATE_RENDER_TARGET;

	s_resourceManager->createTextureRtDs(L"voxelRT", desc);
	return true;
}

bool RenderPassVoxelize::loadShaders()
{
	if (!s_shaderManager->loadShader("VoxelizeVS.cso", L"../../lib/VoxelizeVS.cso", d3d::ShaderType::Vertex))
		return false;

	if (!s_shaderManager->loadShader("VoxelizeGS.cso", L"../../lib/VoxelizeGS.cso", d3d::ShaderType::Geometry))
		return false;

	if (!s_shaderManager->loadShader("VoxelizePS.cso", L"../../lib/VoxelizePS.cso", d3d::ShaderType::Pixel))
		return false;

	return true;
}

bool RenderPassVoxelize::createRootSignature(ID3D12Device* device)
{
	/*
	StructuredBuffer<TransformGpu> s_transforms : register(t0);
	StructuredBuffer<uint> s_materials : register(t1);
	*/
	CD3DX12_DESCRIPTOR_RANGE rangeVS[1];
	rangeVS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0, 0);

	/*
	cbuffer VoxelBuffer : register(b0)
	cbuffer CameraBuffer : register(b1)
	*/
	CD3DX12_DESCRIPTOR_RANGE rangeGS[1];
	rangeGS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0, 0, 2);

	/*
	cbuffer VoxelBuffer : register(b0)
	Texture2DArray g_textureSrgba : register(t2);
	RWTexture3D<uint> g_voxelTextureOpacity : register(u0);
	RWTexture3D<uint> g_voxelTextureDiffuse : register(u1);
	*/
	CD3DX12_DESCRIPTOR_RANGE rangePS[3];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 2);
	rangePS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0, 4);
	rangePS[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, 5);

	CD3DX12_ROOT_PARAMETER rootParameters[3];
	rootParameters[0].InitAsDescriptorTable(1, rangeVS, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsDescriptorTable(1, rangeGS, D3D12_SHADER_VISIBILITY_GEOMETRY);
	rootParameters[2].InitAsDescriptorTable(3, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);

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
	rootSignatureDesc.Init(3, rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	return m_rootSignature.create(&rootSignatureDesc, device);
}

bool RenderPassVoxelize::createPipelineState(ID3D12Device* device)
{
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32_UINT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 }
	};
	
	auto format = DXGI_FORMAT_R8G8B8A8_UNORM;

	d3d::PipelineStateDescInput inputDesc;
	inputDesc.depthEnabled = 0;
	inputDesc.inputLayout.pInputElementDescs = inputLayout;
	inputDesc.inputLayout.NumElements = _countof(inputLayout);
	inputDesc.rootSignature = m_rootSignature.get();
	inputDesc.shaderDesc.vs = s_shaderManager->getShader("VoxelizeVS.cso");
	inputDesc.shaderDesc.gs = s_shaderManager->getShader("VoxelizeGS.cso");
	inputDesc.shaderDesc.ps = s_shaderManager->getShader("VoxelizePS.cso");
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	inputDesc.rtvCount = 1;
	inputDesc.rtvFormats = &format;
	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);
	desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON;
	desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

	desc.RasterizerState.MultisampleEnable = 1;
	desc.SampleDesc.Count = 4;
	desc.SampleDesc.Quality = 0;

	return d3d::PipelineState::create(desc, &m_pipelineState, device);
}

bool RenderPassVoxelize::createDescriptorHeap(ID3D12Device* device)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	if (!m_descriptorHeapClear.create(desc, device))
		return false;

	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NumDescriptors = 6;

	return m_descriptorHeap.create(desc, device);
}

void RenderPassVoxelize::createViews(ID3D12Device* device)
{
	/*
	StructuredBuffer<TransformGpu> s_transforms : register(t0);
	StructuredBuffer<uint> s_materials : register(t1);
	cbuffer VoxelBuffer : register(b0)
	cbuffer CameraBuffer : register(b1)
	Texture2DArray g_textureSrgba : register(t2);
	RWTexture3D<uint> g_voxelTextureOpacity : register(u0);
	RWTexture3D<uint> g_voxelTextureDiffuse : register(u1);
	*/
	
	auto voxelTextureOpacity = s_resourceManager->getTexture(L"voxelTextureOpacity");

	auto cameraBufferView = s_resourceManager->getConstantBufferView("cameraBufferView");

	auto voxelBuffer = s_resourceManager->getBuffer(L"voxelBuffer");
	D3D12_CONSTANT_BUFFER_VIEW_DESC voxelBufferDesc;
	voxelBufferDesc.BufferLocation = voxelBuffer->GetGPUVirtualAddress();
	voxelBufferDesc.SizeInBytes = d3d::AlignedSizeType<GpuVoxel, 1, 256>::size;

	auto handle = m_descriptorHeap.getHandleCpu();
	auto transformBuffer = s_resourceManager->getBuffer(L"transformBuffer");
	auto transformBufferViewDesc = s_resourceManager->getShaderResourceView("transformBufferView");
	device->CreateShaderResourceView(transformBuffer->get(), transformBufferViewDesc, handle);

	auto materialBuffer = s_resourceManager->getBuffer(L"materialBuffer");
	auto materialBufferViewDesc = s_resourceManager->getShaderResourceView("materialBufferView");
	handle.offset(1);
	device->CreateShaderResourceView(materialBuffer->get(), materialBufferViewDesc, handle);

	handle.offset(1);
	device->CreateConstantBufferView(&voxelBufferDesc, handle);

	handle.offset(1);
	device->CreateConstantBufferView(cameraBufferView, handle);

	auto srgbTexture = s_resourceManager->getTexture(L"srgbTexture");
	auto srgbTextureViewDesc = s_resourceManager->getShaderResourceView("srgbTextureView");
	handle.offset(1);
	device->CreateShaderResourceView(srgbTexture->get(), srgbTextureViewDesc, handle);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	uavDesc.Texture3D.FirstWSlice = 0;
	uavDesc.Texture3D.MipSlice = 0;

	uavDesc.Format = DXGI_FORMAT_R32_UINT;
	uavDesc.Texture3D.WSize = s_settings->m_lpvDim;
	handle.offset(1);
	device->CreateUnorderedAccessView(voxelTextureOpacity->get(), nullptr, &uavDesc, handle);

	uavDesc.Format = DXGI_FORMAT_R32_UINT;
	uavDesc.Texture3D.WSize = s_settings->m_lpvDim;
	auto clearHandle = m_descriptorHeapClear.getHandleCpu();
	device->CreateUnorderedAccessView(voxelTextureOpacity->get(), nullptr, &uavDesc, clearHandle);
}

bool RenderPassVoxelize::initialize(ID3D12Device* device, void* p)
{
	if (!loadShaders())
		return false;

	if (!createRootSignature(device))
		return false;

	if (!createPipelineState(device))
		return false;

	if (!createDescriptorHeap(device))
		return false;

	if (!m_commandList.create(device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAlloc->get(), m_pipelineState.get()))
		return false;

	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	if(!m_rtvHeap.create(desc, device))
		return false;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
	rtvDesc.Texture2DMS.UnusedField_NothingToDefine = 0;

	auto voxelRT = s_resourceManager->getTextureRtDs(L"voxelRT");
	device->CreateRenderTargetView(voxelRT->get(), &rtvDesc, m_rtvHeap.getHandleCpu());

	createViews(device);

	return true;
}

void RenderPassVoxelize::shutdown()
{

}

void RenderPassVoxelize::submitCommands(Graphics::CommandQueue* queue)
{
	const u32 clearValues[4] = { 0, 0, 0, 0 };

	if (m_drawCommand->getCount() != 0)
	{
		auto voxelTextureOpacity = s_resourceManager->getTexture(L"voxelTextureOpacity");
		auto voxelTextureColor = s_resourceManager->getTexture(L"voxelTextureColor");

		m_commandList->Reset(m_cmdAlloc->get(), m_pipelineState.get());

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(voxelTextureOpacity->get()));
		auto gpuHandle = m_descriptorHeapClear.getHandleGpu();
		auto cpuHandle = m_descriptorHeapClear.getHandleCpu();
		m_commandList->ClearUnorderedAccessViewUint(gpuHandle, cpuHandle, voxelTextureOpacity->get(), clearValues, 0, nullptr);

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(voxelTextureOpacity->get()));

		D3D12_VIEWPORT viewPort;
		viewPort.Height = (f32)s_settings->m_lpvDim * 4;
		viewPort.Width = (f32)s_settings->m_lpvDim * 4;
		viewPort.MaxDepth = 1.0f;
		viewPort.MinDepth = 0.0f;
		viewPort.TopLeftX = 0;
		viewPort.TopLeftY = 0;

		D3D12_RECT rectScissor;
		rectScissor.left = 0;
		rectScissor.top = 0;
		rectScissor.right = s_settings->m_lpvDim * 4;
		rectScissor.bottom = s_settings->m_lpvDim * 4;

		m_commandList->RSSetViewports(1, &viewPort);
		m_commandList->RSSetScissorRects(1, &rectScissor);

 		D3D12_CPU_DESCRIPTOR_HANDLE rthvHandle = m_rtvHeap.getHandleCpu();
		m_commandList->OMSetRenderTargets(1,&rthvHandle, 0, nullptr);

		auto heap = m_descriptorHeap.get();
		m_commandList->SetDescriptorHeaps(1, &heap);

		m_commandList->SetGraphicsRootSignature(m_rootSignature.get());
		m_commandList->SetGraphicsRootDescriptorTable(0, m_descriptorHeap->GetGPUDescriptorHandleForHeapStart());
		m_commandList->SetGraphicsRootDescriptorTable(1, m_descriptorHeap->GetGPUDescriptorHandleForHeapStart());
		m_commandList->SetGraphicsRootDescriptorTable(2, m_descriptorHeap->GetGPUDescriptorHandleForHeapStart());

		auto drawCmdBuffer = s_resourceManager->getBuffer(L"drawCmdBuffer");
		D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[2];
		vertexBufferViews[0] = s_resourceManager->getResourceView("meshVertexBufferView")->vbv;
		vertexBufferViews[1] = s_resourceManager->getResourceView("meshDrawIdBufferView")->vbv;
		auto indexBufferView = s_resourceManager->getResourceView("meshIndexBufferView")->ibv;

		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_commandList->IASetVertexBuffers(0, 2, vertexBufferViews);
		m_commandList->IASetIndexBuffer(&indexBufferView);

		m_drawCommand->draw(m_commandList.get());

		m_commandList->Close();

		queue->pushCommandList(&m_commandList);
	}
}