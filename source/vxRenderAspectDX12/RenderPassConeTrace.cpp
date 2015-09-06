#include "RenderPassConeTrace.h"
#include "ResourceManager.h"
#include "ShaderManager.h"
#include "GpuVoxel.h"
#include "UploadManager.h"
#include "d3dx12.h"
#include "ResourceView.h"

const u32 g_voxelDim = 128;
const u32 g_voxelDimW = g_voxelDim * 6;

RenderPassConeTrace::RenderPassConeTrace(ID3D12CommandAllocator* cmdAlloc)
	:m_cmdAlloc(cmdAlloc)
{

}

RenderPassConeTrace::~RenderPassConeTrace()
{

}

void RenderPassConeTrace::getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device)
{
	D3D12_RESOURCE_DESC resDesc[1];
	resDesc[0].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc[0].Alignment = 64 KBYTE;
	resDesc[0].Width = s_resolution.x;
	resDesc[0].Height = s_resolution.y;
	resDesc[0].DepthOrArraySize = 1;
	resDesc[0].MipLevels = 1;
	resDesc[0].Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	resDesc[0].SampleDesc.Count = 1;
	resDesc[0].SampleDesc.Quality = 0;
	resDesc[0].Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc[0].Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	auto allocDataTexture = device->GetResourceAllocationInfo(1, 1, &resDesc[0]);

	*heapSizeRtDs += allocDataTexture.SizeInBytes;
}

bool RenderPassConeTrace::createData(ID3D12Device* device)
{
	D3D12_RESOURCE_DESC resDesc[1];
	resDesc[0].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc[0].Alignment = 64 KBYTE;
	resDesc[0].Width = s_resolution.x;
	resDesc[0].Height = s_resolution.y;
	resDesc[0].DepthOrArraySize = 1;
	resDesc[0].MipLevels = 1;
	resDesc[0].Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	resDesc[0].SampleDesc.Count = 1;
	resDesc[0].SampleDesc.Quality = 0;
	resDesc[0].Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc[0].Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	auto allocDataTexture = device->GetResourceAllocationInfo(1, 1, &resDesc[0]);

	D3D12_CLEAR_VALUE clearValue;
	memset(&clearValue, 0, sizeof(clearValue));
	clearValue.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

	CreateResourceDesc desc;
	desc.clearValue = nullptr;
	desc.resDesc = &resDesc[0];
	desc.size = allocDataTexture.SizeInBytes;
	desc.clearValue = &clearValue;
	desc.state = D3D12_RESOURCE_STATE_RENDER_TARGET;
	auto ptr = s_resourceManager->createTextureRtDs(L"indirectTexture", desc);
	if (ptr == nullptr)
		return false;

	return true;
}

bool RenderPassConeTrace::loadShaders()
{
	if (!s_shaderManager->loadShader("ConeTraceVS.cso", L"../../lib/ConeTraceVS.cso", d3d::ShaderType::Vertex))
		return false;

	if (!s_shaderManager->loadShader("ConeTraceGS.cso", L"../../lib/ConeTraceGS.cso", d3d::ShaderType::Geometry))
		return false;

	if (!s_shaderManager->loadShader("ConeTracePS.cso", L"../../lib/ConeTracePS.cso", d3d::ShaderType::Pixel))
		return false;

	return true;
}

bool RenderPassConeTrace::createRootSignature(ID3D12Device* device)
{
	// cbuffer VoxelBuffer : register(b0)
	// cbuffer CameraBuffer : register(b1)
	//	Texture2D<float> g_zBuffer : register(t0);
	// Texture2DArray g_normalSlice : register(t1);
	CD3DX12_DESCRIPTOR_RANGE rangeGS[2];
	rangeGS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0, 0, 0);
	rangeGS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0, 2);

	// cbuffer VoxelBuffer : register(b0)
	// g_voxelTextureDiffuse : register(t2);
	CD3DX12_DESCRIPTOR_RANGE rangePS[2];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 0);
	rangePS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0, 4);

	CD3DX12_ROOT_PARAMETER rootParameters[2];
	rootParameters[0].InitAsDescriptorTable(2, rangeGS, D3D12_SHADER_VISIBILITY_GEOMETRY);
	rootParameters[1].InitAsDescriptorTable(2, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(2, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

bool RenderPassConeTrace::createPipelineState(ID3D12Device* device)
{
	auto rtvFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

	d3d::PipelineStateDescInput inputDesc;
	inputDesc.depthEnabled = 0;
	inputDesc.inputLayout.pInputElementDescs = nullptr;
	inputDesc.inputLayout.NumElements = 0;
	inputDesc.rootSignature = m_rootSignature.get();
	inputDesc.shaderDesc.vs = s_shaderManager->getShader("ConeTraceVS.cso");
	inputDesc.shaderDesc.gs = s_shaderManager->getShader("ConeTraceGS.cso");
	inputDesc.shaderDesc.ps = s_shaderManager->getShader("ConeTracePS.cso");
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	inputDesc.rtvCount = 1;
	inputDesc.rtvFormats = &rtvFormat;
	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);
	desc.BlendState.RenderTarget[0].BlendEnable = 0;
	desc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	desc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	desc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;

	return d3d::PipelineState::create(desc, &m_pipelineState, device);
}

bool RenderPassConeTrace::createDescriptorHeap(ID3D12Device* device)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	if (!m_rtvHeap.create(desc, device))
		return false;

	return true;
}

void RenderPassConeTrace::createRtv(ID3D12Device* device)
{
	auto indirectTexture = s_resourceManager->getTextureRtDs(L"indirectTexture");
	auto handle = m_rtvHeap.getHandleCpu();

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	device->CreateRenderTargetView(indirectTexture, &rtvDesc, handle);
}

bool RenderPassConeTrace::createSrv(ID3D12Device* device)
{
	// cbuffer VoxelBuffer : register(b0)
	// cbuffer CameraBuffer : register(b1)
	//	Texture2D<float> g_zBuffer : register(t0);
	// Texture2DArray g_normalSlice : register(t1);
	// g_voxelTextureDiffuse : register(t2);

	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 5;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (!m_srvHeap.create(desc, device))
		return false;

	auto voxelBuffer = s_resourceManager->getBuffer(L"voxelBuffer");
	auto cameraBufferView = s_resourceManager->getConstantBufferView("cameraBufferView");
	auto zBuffer = s_resourceManager->getTextureRtDs(L"zBuffer0");
	auto gbufferNormal = s_resourceManager->getTextureRtDs(L"gbufferNormal");
	auto voxelTextureDiffuse = s_resourceManager->getTexture(L"voxelTextureDiffuse");


	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = voxelBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = d3d::AlignedSizeType<GpuVoxel, 1, 256>::size;

	auto handle = m_srvHeap.getHandleCpu();
	device->CreateConstantBufferView(&cbvDesc, handle);

	handle.offset(1);
	device->CreateConstantBufferView(cameraBufferView, handle);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescZBuffer;
	srvDescZBuffer.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescZBuffer.Format = DXGI_FORMAT_R32_FLOAT;
	srvDescZBuffer.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDescZBuffer.Texture2D.MipLevels = 1;
	srvDescZBuffer.Texture2D.MostDetailedMip = 0;
	srvDescZBuffer.Texture2D.PlaneSlice = 0;
	srvDescZBuffer.Texture2D.ResourceMinLODClamp = 0;

	handle.offset(1);
	device->CreateShaderResourceView(zBuffer, &srvDescZBuffer, handle);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescNormals;
	srvDescNormals.Format = DXGI_FORMAT_R16G16_FLOAT;
	srvDescNormals.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescNormals.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDescNormals.Texture2DArray.ArraySize = 1;
	srvDescNormals.Texture2DArray.FirstArraySlice = 0;
	srvDescNormals.Texture2DArray.MipLevels = 1;
	srvDescNormals.Texture2DArray.MostDetailedMip = 0;
	srvDescNormals.Texture2DArray.PlaneSlice = 0;
	srvDescNormals.Texture2DArray.ResourceMinLODClamp = 0;

	handle.offset(1);
	device->CreateShaderResourceView(gbufferNormal, &srvDescNormals, handle);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescVoxel;
	srvDescVoxel.Format = DXGI_FORMAT_R32_UINT;
	srvDescVoxel.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescVoxel.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
	srvDescVoxel.Texture3D.MipLevels = 1;
	srvDescVoxel.Texture3D.MostDetailedMip = 0;
	srvDescVoxel.Texture3D.ResourceMinLODClamp = 0;

	handle.offset(1);
	device->CreateShaderResourceView(voxelTextureDiffuse, &srvDescVoxel, handle);

	return true;
}

bool RenderPassConeTrace::initialize(ID3D12Device* device, void* p)
{
	if (!loadShaders())
		return false;

	if (!createRootSignature(device))
		return false;

	if (!createPipelineState(device))
		return false;

	if (!createDescriptorHeap(device))
		return false;

	if (!createSrv(device))
		return true;

	if (!m_commandList.create(device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAlloc, m_pipelineState.get()))
		return false;

	createRtv(device);

	return true;
}

void RenderPassConeTrace::shutdown()
{

}

void RenderPassConeTrace::submitCommands(ID3D12CommandList** list, u32* index)
{
	auto voxelTextureDiffuse = s_resourceManager->getTexture(L"voxelTextureDiffuse");

	const f32 clearValues[4] = { 1, 0, 0, 0 };

	D3D12_VIEWPORT viewPort;
	viewPort.Width = (f32)s_resolution.x;
	viewPort.Height = (f32)s_resolution.y;
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;

	D3D12_RECT rectScissor;
	rectScissor.left = 0;
	rectScissor.top = 0;
	rectScissor.right = s_resolution.x;
	rectScissor.bottom = s_resolution.y;

	m_commandList->Reset(m_cmdAlloc, m_pipelineState.get());

	m_commandList->RSSetViewports(1, &viewPort);
	m_commandList->RSSetScissorRects(1, &rectScissor);

	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvHeap.getHandleCpu();
	m_commandList->OMSetRenderTargets(1,&handle, 0, nullptr);
	m_commandList->ClearRenderTargetView(handle, clearValues, 0, nullptr);

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	auto heap = m_srvHeap.get();
	m_commandList->SetDescriptorHeaps(1, &heap);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(voxelTextureDiffuse, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	m_commandList->SetGraphicsRootSignature(m_rootSignature.get());
	m_commandList->SetGraphicsRootDescriptorTable(0, heap->GetGPUDescriptorHandleForHeapStart());
	m_commandList->SetGraphicsRootDescriptorTable(1, heap->GetGPUDescriptorHandleForHeapStart());

	m_commandList->DrawInstanced(s_resolution.x, s_resolution.y, 0, 0);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(voxelTextureDiffuse, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	m_commandList->Close();

	list[*index] = m_commandList.get();
	++(*index);

	/*if (m_drawCommand->getCount() != 0)
	{
		auto voxelTextureOpacity = s_resourceManager->getTexture(L"voxelTextureOpacity");
		auto voxelTextureDiffuse = s_resourceManager->getTexture(L"voxelTextureDiffuse");

		m_commandList->Reset(m_cmdAlloc, m_pipelineState.get());

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(voxelTextureOpacity));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(voxelTextureDiffuse));
		auto gpuHandle = m_descriptorHeapClear.getHandleGpu();
		auto cpuHandle = m_descriptorHeapClear.getHandleCpu();
		m_commandList->ClearUnorderedAccessViewUint(gpuHandle, cpuHandle, voxelTextureOpacity, clearValues, 0, nullptr);

		cpuHandle.offset(1);
		gpuHandle.offset(1);
		m_commandList->ClearUnorderedAccessViewUint(gpuHandle, cpuHandle, voxelTextureDiffuse, clearValues, 0, nullptr);
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(voxelTextureDiffuse));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(voxelTextureOpacity));



		m_commandList->RSSetViewports(1, &viewPort);
		m_commandList->RSSetScissorRects(1, &rectScissor);

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

		list[*index] = m_commandList.get();
		++(*index);
	}*/
}