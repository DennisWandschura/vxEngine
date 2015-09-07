#include "RenderPassVoxelize.h"
#include "ResourceManager.h"
#include "ShaderManager.h"
#include "GpuVoxel.h"
#include "UploadManager.h"
#include "d3dx12.h"
#include "ResourceView.h"

const u32 g_voxelDim = 64;
const u32 g_voxelDimW = g_voxelDim * 6;

RenderPassVoxelize::RenderPassVoxelize(ID3D12CommandAllocator* cmdAlloc, DrawIndexedIndirectCommand* drawCommand)
	:m_cmdAlloc(cmdAlloc),
	m_drawCommand(drawCommand)
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
	resDesc[0].DepthOrArraySize = g_voxelDimW;
	resDesc[0].MipLevels = 3;
	resDesc[0].Format = DXGI_FORMAT_R32_TYPELESS;
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

	*heapSizeTexture += allocDataTexture.SizeInBytes * 2;
	*heapSizeBuffer += allocDataBuffer.SizeInBytes;
}

bool RenderPassVoxelize::createData(ID3D12Device* device)
{
	D3D12_RESOURCE_DESC resDesc[1];
	resDesc[0].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
	resDesc[0].Alignment = 64 KBYTE;
	resDesc[0].Width = g_voxelDim;
	resDesc[0].Height = g_voxelDim;
	resDesc[0].DepthOrArraySize = g_voxelDimW;
	resDesc[0].MipLevels = 3;
	resDesc[0].Format = DXGI_FORMAT_R32_TYPELESS;
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
	auto ptr = s_resourceManager->createTexture(L"voxelTextureOpacity", desc);
	if (ptr == nullptr)
		return false;

	resDesc[0].Format = DXGI_FORMAT_R32_UINT;
	ptr = s_resourceManager->createTexture(L"voxelTextureDiffuse", desc);
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
	rangeGS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0, 0, 1);

	CD3DX12_DESCRIPTOR_RANGE rangePS[2];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 1);
	rangePS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 2, 0, 0, 3);

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
	desc.NumDescriptors = 2;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	if (!m_descriptorHeapClear.create(desc, device))
		return false;

	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NumDescriptors = 5;

	return m_descriptorHeap.create(desc, device);
}

void RenderPassVoxelize::createViews(ID3D12Device* device)
{
	auto voxelBuffer = s_resourceManager->getBuffer(L"voxelBuffer");
	auto transformBuffer = s_resourceManager->getBuffer(L"transformBuffer");
	auto voxelTextureOpacity = s_resourceManager->getTexture(L"voxelTextureOpacity");
	auto voxelTextureDiffuse = s_resourceManager->getTexture(L"voxelTextureDiffuse");

	auto transformBufferViewDesc = s_resourceManager->getShaderResourceView("transformBufferView");
	auto cameraBufferView = s_resourceManager->getConstantBufferView("cameraBufferView");

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbufferDesc;
	cbufferDesc.BufferLocation = voxelBuffer->GetGPUVirtualAddress();
	cbufferDesc.SizeInBytes = d3d::AlignedSizeType<GpuVoxel, 1, 256>::size;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	uavDesc.Texture3D.FirstWSlice = 0;
	uavDesc.Texture3D.MipSlice = 0;
	uavDesc.Texture3D.WSize = g_voxelDimW;

	auto handle = m_descriptorHeap.getHandleCpu();
	device->CreateShaderResourceView(transformBuffer, transformBufferViewDesc, handle);

	handle.offset(1);
	device->CreateConstantBufferView(&cbufferDesc, handle);

	handle.offset(1);
	device->CreateConstantBufferView(cameraBufferView, handle);

	uavDesc.Format = DXGI_FORMAT_R32_FLOAT;
	handle.offset(1);
	device->CreateUnorderedAccessView(voxelTextureOpacity, nullptr, &uavDesc, handle);

	uavDesc.Format = DXGI_FORMAT_R32_UINT;
	handle.offset(1);
	device->CreateUnorderedAccessView(voxelTextureDiffuse, nullptr, &uavDesc, handle);

	uavDesc.Format = DXGI_FORMAT_R32_FLOAT;
	auto clearHandle = m_descriptorHeapClear.getHandleCpu();
	device->CreateUnorderedAccessView(voxelTextureOpacity, nullptr, &uavDesc, clearHandle);

	uavDesc.Format = DXGI_FORMAT_R32_UINT;
	clearHandle.offset(1);
	device->CreateUnorderedAccessView(voxelTextureDiffuse, nullptr, &uavDesc, clearHandle);
}

void RenderPassVoxelize::uploadBufferData()
{
	GpuVoxel data;

	const f32 gridsize = 16.0f;

	auto halfDim = g_voxelDim / 2;
	auto gridHalfSize = gridsize / 2.0f;

	auto gridCellSize = gridHalfSize / halfDim;
	auto invGridCellSize = 1.0f / gridCellSize;

	const __m128 axisY = {0, 1, 0, 0};
	const __m128 axisX = { 1, 0, 0, 0 };

	auto projectionMatrix = vx::MatrixOrthographicRHDX(gridsize, gridsize, 0.0f, gridsize);
	auto backFront = projectionMatrix * vx::MatrixTranslation(0, 0, -gridHalfSize);
	auto leftRight = projectionMatrix * vx::MatrixRotationAxis(axisY, vx::degToRad(90)) * vx::MatrixTranslation(gridHalfSize, 0, 0);
	auto upDown = projectionMatrix * vx::MatrixTranslation(0, 0, -gridHalfSize) * vx::MatrixRotationAxis(axisX, vx::degToRad(90));

	data.dim = g_voxelDim;
	data.halfDim = halfDim;
	data.invGridCellSize = invGridCellSize;
	data.projectionMatrix[0] = leftRight;
	data.projectionMatrix[1] = upDown;
	data.projectionMatrix[2] = backFront;
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
	const u32 clearValues[4] = { 0, 0, 0, 0 };

	if (m_drawCommand->getCount() != 0)
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

		D3D12_VIEWPORT viewPort;
		viewPort.Height = (f32)g_voxelDim * 2;
		viewPort.Width = (f32)g_voxelDim * 2;
		viewPort.MaxDepth = 1.0f;
		viewPort.MinDepth = 0.0f;
		viewPort.TopLeftX = 0;
		viewPort.TopLeftY = 0;

		D3D12_RECT rectScissor;
		rectScissor.left = 0;
		rectScissor.top = 0;
		rectScissor.right = g_voxelDim * 2;
		rectScissor.bottom = g_voxelDim * 2;

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
	}
}