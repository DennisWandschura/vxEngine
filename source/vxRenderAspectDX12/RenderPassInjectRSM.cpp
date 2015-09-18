#include "RenderPassInjectRSM.h"
#include "d3dx12.h"
#include "ShaderManager.h"
#include "CommandAllocator.h"
#include "ResourceManager.h"
#include "GpuVoxel.h"

RenderPassInjectRSM::RenderPassInjectRSM(d3d::CommandAllocator* allocator)
	:m_allocator(allocator),
	m_commandList(),
	m_lightCount(0)
{

}

RenderPassInjectRSM::~RenderPassInjectRSM()
{
}

bool RenderPassInjectRSM::loadShaders()
{
	if (!s_shaderManager->loadShader("InjectRsmVoxelVS.cso", L"../../lib/InjectRsmVoxelVS.cso", d3d::ShaderType::Vertex))
		return false;

	if (!s_shaderManager->loadShader("InjectRsmVoxelPS.cso", L"../../lib/InjectRsmVoxelPS.cso", d3d::ShaderType::Pixel))
		return false;

	return true;
}

bool RenderPassInjectRSM::createRootSignature(ID3D12Device* device)
{
	/*
	cbuffer VoxelBuffer : register(b0)

Texture2DArray<float4> g_color : register(t0);
Texture2DArray<half4> g_normals : register(t1);
Texture2DArray<float> g_depth : register(t2);

StructuredBuffer<GpuShadowTransformReverse> g_transforms : register(t3);*/

	CD3DX12_DESCRIPTOR_RANGE rangeVS[2];
	rangeVS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 0);
	rangeVS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0, 0, 1);

	CD3DX12_DESCRIPTOR_RANGE rangePS[1];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 2, 0, 0, 0);

	CD3DX12_ROOT_PARAMETER rootParameters[3];
	rootParameters[0].InitAsDescriptorTable(2, rangeVS, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsConstants(2, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX);
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

bool RenderPassInjectRSM::createPipelineState(ID3D12Device* device)
{
	d3d::PipelineStateDescInput inputDesc;
	inputDesc.inputLayout.pInputElementDescs = nullptr;
	inputDesc.inputLayout.NumElements = 0;
	inputDesc.depthEnabled = 0;
	inputDesc.rootSignature = m_rootSignature.get();
	inputDesc.shaderDesc.vs = s_shaderManager->getShader("InjectRsmVoxelVS.cso");
	inputDesc.shaderDesc.ps = s_shaderManager->getShader("InjectRsmVoxelPS.cso");
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	inputDesc.rtvCount = 0;

	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);

	return d3d::PipelineState::create(desc, &m_pipelineState, device);
}

void RenderPassInjectRSM::getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device)
{

}

bool RenderPassInjectRSM::createData(ID3D12Device* device)
{
	return true;
}

bool RenderPassInjectRSM::createSrv(ID3D12Device* device)
{
	/*
	Texture2DArray<float4> g_color : register(t0);
Texture2DArray<half4> g_normals : register(t1);
Texture2DArray<float> g_depth : register(t2);
StructuredBuffer<GpuShadowTransformReverse> g_transforms : register(t3);
*/
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 5 + 6 * 2;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	if (!m_srvHeap.create(desc, device))
		return false;

	m_srvHeap->SetName(L"RenderPassInjectRSM_SrvHeap");

	auto voxelBuffer = s_resourceManager->getBuffer(L"voxelBuffer");
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = voxelBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = d3d::AlignedSizeType<GpuVoxel, 1, 256>::size;
	
	auto handle = m_srvHeap.getHandleCpu();
	device->CreateConstantBufferView(&cbvDesc, handle);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.ArraySize = s_settings->m_shadowCastingLightCount * 6;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.MipLevels = 1;
	srvDesc.Texture2DArray.MostDetailedMip = 1;
	srvDesc.Texture2DArray.PlaneSlice = 0;
	srvDesc.Texture2DArray.ResourceMinLODClamp = 0;

	auto rsmFilteredColor = s_resourceManager->getTextureRtDs(L"rsmFilteredColor");
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	handle.offset(1);
	device->CreateShaderResourceView(rsmFilteredColor->get(), &srvDesc, handle);

	auto rsmFilteredNormal = s_resourceManager->getTextureRtDs(L"rsmFilteredNormal");
	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

	handle.offset(1);
	device->CreateShaderResourceView(rsmFilteredNormal->get(), &srvDesc, handle);

	auto rsmFilteredDepth = s_resourceManager->getTextureRtDs(L"rsmFilteredDepth");
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;

	handle.offset(1);
	device->CreateShaderResourceView(rsmFilteredDepth->get(), &srvDesc, handle);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	uavDesc.Texture3D.MipSlice = 0;
	uavDesc.Texture3D.WSize = s_settings->m_lpvDim;
	uavDesc.Format = DXGI_FORMAT_R32_UINT;

	handle.offset(1);

	auto voxelTextureOpacity = s_resourceManager->getTexture(L"voxelTextureOpacity");
	auto voxelTextureColor = s_resourceManager->getTexture(L"voxelTextureColor");
	for (u32 i = 0; i < 6; ++i)
	{
		uavDesc.Texture3D.FirstWSlice = 0;

		device->CreateUnorderedAccessView(voxelTextureOpacity->get(), nullptr, &uavDesc, handle);
		handle.offset(1);

		uavDesc.Texture3D.FirstWSlice = i * s_settings->m_lpvDim;
		device->CreateUnorderedAccessView(voxelTextureColor->get(), nullptr, &uavDesc, handle);
		handle.offset(1);
	}

	return true;
}

bool RenderPassInjectRSM::createUav(ID3D12Device* device)
{
	/*D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 6 * 2;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	if (!m_uavHeap.create(desc, device))
		return false;

	m_uavHeap->SetName(L"RenderPassInjectRSM_UavHeap");

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	uavDesc.Texture3D.MipSlice = 0;
	uavDesc.Texture3D.WSize = s_settings->m_lpvDim;
	uavDesc.Format = DXGI_FORMAT_R32_UINT;

	auto handle = m_uavHeap.getHandleCpu();

	auto voxelTextureOpacity = s_resourceManager->getTexture(L"voxelTextureOpacity");
	auto voxelTextureColor = s_resourceManager->getTexture(L"voxelTextureColor");
	for (u32 i = 0; i < 6; ++i)
	{
		uavDesc.Texture3D.FirstWSlice = i * s_settings->m_lpvDim;

		device->CreateUnorderedAccessView(voxelTextureOpacity->get(), nullptr, &uavDesc, handle);
		handle.offset(1);

		device->CreateUnorderedAccessView(voxelTextureColor->get(), nullptr, &uavDesc, handle);
		handle.offset(1);
	}*/

	return true;
}

bool RenderPassInjectRSM::createUavClear(ID3D12Device* device)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 2;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	if (!m_uavClearHeap.create(desc, device))
		return false;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	uavDesc.Texture3D.MipSlice = 0;
	uavDesc.Texture3D.WSize = s_settings->m_lpvDim;
	uavDesc.Texture3D.FirstWSlice = 0;
	uavDesc.Format = DXGI_FORMAT_R32_UINT;

	auto handle = m_uavClearHeap.getHandleCpu();

	auto voxelTextureOpacity = s_resourceManager->getTexture(L"voxelTextureOpacity");
	auto voxelTextureColor = s_resourceManager->getTexture(L"voxelTextureColor");

	device->CreateUnorderedAccessView(voxelTextureOpacity->get(), nullptr, &uavDesc, handle);
	handle.offset(1);

	uavDesc.Texture3D.WSize = s_settings->m_lpvDim * 6;
	device->CreateUnorderedAccessView(voxelTextureColor->get(), nullptr, &uavDesc, handle);

	return true;
}

bool RenderPassInjectRSM::initialize(ID3D12Device* device, void* p)
{
	if (!loadShaders())
		return false;

	if (!createRootSignature(device))
	{
		return false;
	}

	if (!createPipelineState(device))
	{
		return false;
	}

	if (!createSrv(device))
		return false;

	if (!createUav(device))
		return false;

	if (!createUavClear(device))
		return false;

	if (!m_commandList.create(device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_allocator->get(), m_pipelineState.get()))
		return false;

	return true;
}

void RenderPassInjectRSM::shutdown()
{

}

void RenderPassInjectRSM::submitCommands(Graphics::CommandQueue* queue)
{
	const u32 clearValues[4] = {0, 0, 0,0};
	if (m_lightCount != 0)
	{
		u32 textureDim = s_settings->m_shadowDim / 4;

		auto voxelTextureOpacity = s_resourceManager->getTexture(L"voxelTextureOpacity");
		auto voxelTextureColor = s_resourceManager->getTexture(L"voxelTextureColor");

		m_commandList->Reset(m_allocator->get(), m_pipelineState.get());

		auto clearHandleGpu = m_uavClearHeap.getHandleGpu();
		auto clearHandleCpu = m_uavClearHeap.getHandleCpu();
		m_commandList->ClearUnorderedAccessViewUint(clearHandleGpu, clearHandleCpu, voxelTextureOpacity->get(), clearValues, 0, nullptr);

		clearHandleGpu.offset(1);
		clearHandleCpu.offset(1);
		m_commandList->ClearUnorderedAccessViewUint(clearHandleGpu, clearHandleCpu, voxelTextureColor->get(), clearValues, 0, nullptr);

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(voxelTextureOpacity->get()));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(voxelTextureColor->get()));

		ID3D12DescriptorHeap* heaps[1] = 
		{
			m_srvHeap.get()
		};

		D3D12_VIEWPORT viewport;
		viewport.Height = (f32)textureDim;
		viewport.Width = (f32)textureDim;
		viewport.MaxDepth = 1.0f;
		viewport.MinDepth = 0.0f;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;

		D3D12_RECT rectScissor;
		rectScissor.left = 0;
		rectScissor.top = 0;
		rectScissor.right = textureDim;
		rectScissor.bottom = textureDim;

		m_commandList->RSSetScissorRects(1, &rectScissor);
		m_commandList->RSSetViewports(1, &viewport);
		
		m_commandList->SetDescriptorHeaps(1, heaps);
		m_commandList->SetGraphicsRootSignature(m_rootSignature.get());

		m_commandList->SetGraphicsRootDescriptorTable(0, m_srvHeap->GetGPUDescriptorHandleForHeapStart());

		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		for (u32 lightIndex = 0; lightIndex < m_lightCount; ++lightIndex)
		{
			auto uavTargetHandleGpu = m_srvHeap.getHandleGpu();
			uavTargetHandleGpu.offset(5);

			for (u32 i = 0; i < 6; ++i)
			{
				//uint lightIndex;
				//uint cubeIndex;
				vx::uint2 rootData;
				rootData.x = lightIndex;
				rootData.y = i;

				m_commandList->SetGraphicsRoot32BitConstants(1, 2, &rootData, 0);
				m_commandList->SetGraphicsRootDescriptorTable(2, uavTargetHandleGpu);

				m_commandList->DrawInstanced(textureDim, textureDim, 0, 0);

				uavTargetHandleGpu.offset(2);
			}
		}

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(voxelTextureOpacity->get()));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(voxelTextureColor->get()));

		m_commandList->Close();
		queue->pushCommandList(&m_commandList);
	}
}