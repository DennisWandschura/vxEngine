#include "RenderPassVoxelize.h"
#include "ResourceManager.h"
#include "ShaderManager.h"
#include "GpuVoxel.h"
#include "UploadManager.h"
#include "d3dx12.h"
#include "ResourceView.h"

const u32 g_voxelDim = 128;

RenderPassVoxelize::RenderPassVoxelize(ID3D12CommandAllocator* cmdAlloc, u32 countOffset)
	:m_cmdAlloc(cmdAlloc),
	m_drawCount(0),
	m_countOffset(countOffset)
{

}

RenderPassVoxelize::~RenderPassVoxelize()
{

}

void RenderPassVoxelize::getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device)
{
	D3D12_RESOURCE_DESC resDesc[2];
	resDesc[0].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
	resDesc[0].Alignment = 64 KBYTE;
	resDesc[0].Width = g_voxelDim;
	resDesc[0].Height = g_voxelDim;
	resDesc[0].DepthOrArraySize = g_voxelDim * 2;
	resDesc[0].MipLevels = 1;
	resDesc[0].Format = DXGI_FORMAT_R32_UINT;
	resDesc[0].SampleDesc.Count = 1;
	resDesc[0].SampleDesc.Quality = 0;
	resDesc[0].Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc[0].Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	resDesc[1].Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc[1].Alignment = 64 KBYTE;
	resDesc[1].Width = 64 KBYTE;
	resDesc[1].Height = 1;
	resDesc[1].DepthOrArraySize = 1;
	resDesc[1].MipLevels = 1;
	resDesc[1].Format = DXGI_FORMAT_UNKNOWN;
	resDesc[1].SampleDesc.Count = 1;
	resDesc[1].SampleDesc.Quality = 0;
	resDesc[1].Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resDesc[1].Flags = D3D12_RESOURCE_FLAG_NONE;

	auto allocDataTexture = device->GetResourceAllocationInfo(1, 1, &resDesc[0]);
	auto allocDataBuffer = device->GetResourceAllocationInfo(1, 1, &resDesc[1]);

	*heapSizeTexture += allocDataTexture.SizeInBytes;
	*heapSizeBuffer += allocDataBuffer.SizeInBytes;
}

bool RenderPassVoxelize::createData(ID3D12Device* device)
{
	D3D12_RESOURCE_DESC resDesc[1];
	resDesc[0].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
	resDesc[0].Alignment = 64 KBYTE;
	resDesc[0].Width = g_voxelDim;
	resDesc[0].Height = g_voxelDim;
	resDesc[0].DepthOrArraySize = g_voxelDim * 2;
	resDesc[0].MipLevels = 1;
	resDesc[0].Format = DXGI_FORMAT_R32_UINT;
	resDesc[0].SampleDesc.Count = 1;
	resDesc[0].SampleDesc.Quality = 0;
	resDesc[0].Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc[0].Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	auto allocDataTexture = device->GetResourceAllocationInfo(1, 1, &resDesc[0]);

	CreateResourceDesc desc;
	desc.clearValue = nullptr;
	desc.resDesc = &resDesc[0];
	desc.size = allocDataTexture.SizeInBytes;
	desc.state = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	auto ptr = s_resourceManager->createTexture(L"voxelTexture", desc);
	if (ptr == nullptr)
		return false;

	auto buffer = s_resourceManager->createBuffer(L"voxelBuffer", 64 KBYTE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	if (buffer == nullptr)
		return false;

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
	CD3DX12_DESCRIPTOR_RANGE rangeVS[1];
	rangeVS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, 0);

	CD3DX12_DESCRIPTOR_RANGE rangeGS[1];
	rangeGS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 1);

	CD3DX12_DESCRIPTOR_RANGE rangePS[2];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 1);
	rangePS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, 2);

	CD3DX12_ROOT_PARAMETER rootParameters[3];
	rootParameters[0].InitAsDescriptorTable(1, rangeVS, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsDescriptorTable(1, rangeGS, D3D12_SHADER_VISIBILITY_GEOMETRY);
	rootParameters[2].InitAsDescriptorTable(2, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);

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
	
	d3d::PipelineStateDescInput inputDesc;
	inputDesc.depthEnabled = 0;
	inputDesc.inputLayout.pInputElementDescs = inputLayout;
	inputDesc.inputLayout.NumElements = _countof(inputLayout);
	inputDesc.rootSignature = m_rootSignature.get();
	inputDesc.shaderDesc.vs = s_shaderManager->getShader("VoxelizeVS.cso");
	inputDesc.shaderDesc.gs = s_shaderManager->getShader("VoxelizeGS.cso");
	inputDesc.shaderDesc.ps = s_shaderManager->getShader("VoxelizePS.cso");
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);
	desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON;
	desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

	return d3d::PipelineState::create(desc, &m_pipelineState, device);
}

bool RenderPassVoxelize::createDescriptorHeap(ID3D12Device* device)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;;
	desc.NodeMask = 1;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	if (!m_descriptorHeapClear.create(desc, device))
		return false;

	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NumDescriptors = 3;

	return m_descriptorHeap.create(desc, device);
}

void RenderPassVoxelize::createViews(ID3D12Device* device)
{
	auto voxelBuffer = s_resourceManager->getBuffer(L"voxelBuffer");
	auto transformBuffer = s_resourceManager->getBuffer(L"transformBuffer");
	auto voxelTexture = s_resourceManager->getTexture(L"voxelTexture");

	auto transformBufferViewDesc = s_resourceManager->getShaderResourceView("transformBufferView");

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbufferDesc;
	cbufferDesc.BufferLocation = voxelBuffer->GetGPUVirtualAddress();
	cbufferDesc.SizeInBytes = d3d::AlignedSizeType<GpuVoxel, 1, 256>::size;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	uavDesc.Format = DXGI_FORMAT_R32_UINT;
	uavDesc.Texture3D.FirstWSlice = 0;
	uavDesc.Texture3D.MipSlice = 0;
	uavDesc.Texture3D.WSize = g_voxelDim * 2;

	auto handle = m_descriptorHeap.getHandleCpu();
	device->CreateShaderResourceView(transformBuffer, transformBufferViewDesc, handle);

	handle.offset(1);
	device->CreateConstantBufferView(&cbufferDesc, handle);

	handle.offset(1);
	device->CreateUnorderedAccessView(voxelTexture, nullptr, &uavDesc, handle);

	device->CreateUnorderedAccessView(voxelTexture, nullptr, &uavDesc, m_descriptorHeapClear.getHandleCpu());
}

void RenderPassVoxelize::uploadBufferData()
{
	GpuVoxel data;

	const f32 gridsize = 8.0f;

	auto halfDim = g_voxelDim / 2;
	auto gridHalfSize = gridsize / 2.0f;

	auto gridCellSize = gridHalfSize / halfDim;
	auto invGridCellSize = 1.0f / gridCellSize;

	auto projectionMatrix = vx::MatrixOrthographicRHDX(gridsize, gridsize, 0.0f, gridsize);
	auto backFront = projectionMatrix * vx::MatrixTranslation(0, 0, -gridHalfSize);

	data.dim = g_voxelDim;
	data.halfDim = halfDim;
	data.invGridCellSize = invGridCellSize;
	data.projectionMatrix = backFront;
	data.gridCellSize = gridCellSize;

	auto voxelBuffer = s_resourceManager->getBuffer(L"voxelBuffer");
	s_uploadManager->pushUploadBuffer((u8*)&data, voxelBuffer, 0, sizeof(GpuVoxel), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
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

	D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[1] = {};
	argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

	D3D12_COMMAND_SIGNATURE_DESC cmdSigDesc;
	cmdSigDesc.ByteStride = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
	cmdSigDesc.NodeMask = 0;
	cmdSigDesc.NumArgumentDescs = 1;
	cmdSigDesc.pArgumentDescs = argumentDescs;
	auto hresult = device->CreateCommandSignature(&cmdSigDesc, nullptr, IID_PPV_ARGS(m_commandSignature.getAddressOf()));
	if (hresult != 0)
		return false;

	if (!m_commandList.create(device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAlloc, m_pipelineState.get()))
		return false;

	createViews(device);

	uploadBufferData();

	return true;
}

void RenderPassVoxelize::shutdown()
{

}

void RenderPassVoxelize::submitCommands(ID3D12CommandList** list, u32* index)
{
	auto voxelTexture = s_resourceManager->getTexture(L"voxelTexture");

	m_commandList->Reset(m_cmdAlloc, m_pipelineState.get());

	const u32 clearValues[4] = { 0, 0, 0, 0 };

	m_commandList->ClearUnorderedAccessViewUint(m_descriptorHeapClear->GetGPUDescriptorHandleForHeapStart(), m_descriptorHeapClear->GetCPUDescriptorHandleForHeapStart(), voxelTexture, clearValues, 0, nullptr);

	if (m_drawCount != 0)
	{
		D3D12_VIEWPORT viewPort;
		viewPort.Height = (f32)g_voxelDim;
		viewPort.Width = (f32)g_voxelDim;
		viewPort.MaxDepth = 1.0f;
		viewPort.MinDepth = 0.0f;
		viewPort.TopLeftX = 0;
		viewPort.TopLeftY = 0;

		D3D12_RECT rectScissor;
		rectScissor.left = 0;
		rectScissor.top = 0;
		rectScissor.right = g_voxelDim;
		rectScissor.bottom = g_voxelDim;

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

		m_commandList->ExecuteIndirect(m_commandSignature.get(), m_drawCount, drawCmdBuffer, 0, drawCmdBuffer, m_countOffset);
	}

	m_commandList->Close();

	list[*index] = m_commandList.get();
	++(*index);
}