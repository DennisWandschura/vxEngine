#include "RenderPassInjectRSM.h"
#include "d3dx12.h"
#include "ShaderManager.h"
#include "CommandAllocator.h"
#include "ResourceManager.h"
#include "GpuVoxel.h"
#include "ResourceDesc.h"

RenderPassInjectRSM::RenderPassInjectRSM(d3d::CommandAllocator* allocator)
	:RenderPassLight(),
	m_allocator(allocator),
	m_commandList(),
	m_buildList(0)
{

}

RenderPassInjectRSM::~RenderPassInjectRSM()
{
}

bool RenderPassInjectRSM::loadShaders()
{
	if (!s_shaderManager->loadShader(L"InjectRsmVoxelVS.cso"))
		return false;

	if (!s_shaderManager->loadShader(L"InjectRsmVoxelPS.cso"))
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
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1, 0, 5);

	CD3DX12_ROOT_PARAMETER rootParameters[3];
	rootParameters[0].InitAsDescriptorTable(2, rangeVS, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsConstants(2, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[2].InitAsDescriptorTable(1, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(3, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	return m_rootSignature.create(&rootSignatureDesc, device);
}

bool RenderPassInjectRSM::createPipelineState(ID3D12Device* device)
{
	//auto rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	d3d::PipelineStateDescInput inputDesc;
	inputDesc.inputLayout.pInputElementDescs = nullptr;
	inputDesc.inputLayout.NumElements = 0;
	inputDesc.depthEnabled = 0;
	inputDesc.rootSignature = m_rootSignature.get();
	inputDesc.shaderDesc.vs = s_shaderManager->getShader(L"InjectRsmVoxelVS.cso");
	inputDesc.shaderDesc.ps = s_shaderManager->getShader(L"InjectRsmVoxelPS.cso");
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	inputDesc.rtvCount = 0;
	//inputDesc.rtvFormats = &rtvFormat;

	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);

	return d3d::PipelineState::create(desc, &m_pipelineState, device);
}

void RenderPassInjectRSM::getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device)
{
	vx::uint2 resolution = vx::uint2(s_settings->m_shadowDim) / 4;
	auto resDesc = d3d::ResourceDesc::getDescTexture2D(resolution, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
	auto allocInfo = device->GetResourceAllocationInfo(1, 1, &resDesc);

	*heapSizeRtDs += allocInfo.SizeInBytes;
}

bool RenderPassInjectRSM::createData(ID3D12Device* device)
{
	vx::uint2 resolution = vx::uint2(s_settings->m_shadowDim) / 4;
	auto resDesc = d3d::ResourceDesc::getDescTexture2D(resolution, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
	auto allocInfo = device->GetResourceAllocationInfo(1, 1, &resDesc);

	D3D12_CLEAR_VALUE clearValue;
	memset(&clearValue, 0, sizeof(clearValue));
	clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	CreateResourceDesc desc = CreateResourceDesc::createDesc(allocInfo.SizeInBytes, &resDesc,&clearValue, D3D12_RESOURCE_STATE_RENDER_TARGET);
	auto ptr = s_resourceManager->createTextureRtDs(L"injectRsmDebug", desc);
	if (ptr == nullptr)
		return false;

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
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.PlaneSlice = 0;
	srvDesc.Texture2DArray.ResourceMinLODClamp = 0;

	auto rsmFilteredColor = s_resourceManager->getTextureRtDs(L"shadowTextureColor");
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	handle.offset(1);
	device->CreateShaderResourceView(rsmFilteredColor->get(), &srvDesc, handle);

	auto rsmFilteredNormal = s_resourceManager->getTextureRtDs(L"shadowTextureNormal");
	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

	handle.offset(1);
	device->CreateShaderResourceView(rsmFilteredNormal->get(), &srvDesc, handle);

	auto rsmFilteredDepth = s_resourceManager->getTextureRtDs(L"shadowTextureDepth");
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;

	handle.offset(1);
	device->CreateShaderResourceView(rsmFilteredDepth->get(), &srvDesc, handle);

	auto shadowReverseTransformBufferView = s_resourceManager->getShaderResourceView("shadowReverseTransformBufferView");
	auto shadowReverseTransformBuffer = s_resourceManager->getBuffer(L"shadowReverseTransformBuffer");
	handle.offset(1);
	device->CreateShaderResourceView(shadowReverseTransformBuffer->get(), shadowReverseTransformBufferView, handle);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	uavDesc.Texture3D.MipSlice = 0;
	uavDesc.Format = DXGI_FORMAT_R32_UINT;
	uavDesc.Texture3D.FirstWSlice = 0;
	uavDesc.Texture3D.WSize = s_settings->m_lpvDim * 6;

	auto voxelTextureColor = s_resourceManager->getTexture(L"voxelTextureColor");
	handle.offset(1);
	device->CreateUnorderedAccessView(voxelTextureColor->get(), nullptr, &uavDesc, handle);

	return true;
}

bool RenderPassInjectRSM::createUav(ID3D12Device* device)
{
	return true;
}

bool RenderPassInjectRSM::createUavClear(ID3D12Device* device)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 1;
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

	auto voxelTextureColor = s_resourceManager->getTexture(L"voxelTextureColor");

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

	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	if (!m_rtvHeap.create(desc, device))
		return false;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;
	auto injectRsmDebug = s_resourceManager->getTextureRtDs(L"injectRsmDebug");
	device->CreateRenderTargetView(injectRsmDebug->get(), &rtvDesc, m_rtvHeap.getHandleCpu());

	return true;
}

void RenderPassInjectRSM::shutdown()
{

}

void RenderPassInjectRSM::buildCommands()
{
	const u32 clearValues[4] = { 0, 0, 0,0 };
	if (m_visibleLightCount != 0)
	{
		u32 textureDim = s_settings->m_shadowDim;

		auto voxelTextureColor = s_resourceManager->getTexture(L"voxelTextureColor");
		auto shadowReverseTransformBuffer = s_resourceManager->getBuffer(L"shadowReverseTransformBuffer");

		m_commandList->Reset(m_allocator->get(), m_pipelineState.get());

		//auto clearHandleGpu = m_uavClearHeap.getHandleGpu();
		//auto clearHandleCpu = m_uavClearHeap.getHandleCpu();

		//m_commandList->ClearUnorderedAccessViewUint(clearHandleGpu, clearHandleCpu, voxelTextureColor->get(), clearValues, 0, nullptr);

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(voxelTextureColor->get()));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowReverseTransformBuffer->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

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

		//D3D12_CPU_DESCRIPTOR_HANDLE rtvHanlde = m_rtvHeap.getHandleCpu();
		//m_commandList->OMSetRenderTargets(1, &rtvHanlde, 0, nullptr);

		m_commandList->SetDescriptorHeaps(1, heaps);
		m_commandList->SetGraphicsRootSignature(m_rootSignature.get());

		m_commandList->SetGraphicsRootDescriptorTable(0, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
		m_commandList->SetGraphicsRootDescriptorTable(2, m_srvHeap->GetGPUDescriptorHandleForHeapStart());

		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		//auto rsmFilteredColor = s_resourceManager->getTextureRtDs(L"shadowTextureColor");
	//	auto rsmFilteredNormal = s_resourceManager->getTextureRtDs(L"shadowTextureNormal");
		//auto rsmFilteredDepth = s_resourceManager->getTextureRtDs(L"shadowTextureDepth");

		for (u32 lightIndex = 0; lightIndex < m_visibleLightCount; ++lightIndex)
		{
			for (u32 i = 0; i < 6; ++i)
			{
				//uint lightIndex;
				//uint cubeIndex;
				vx::uint2 rootData;
				rootData.x = lightIndex;
				rootData.y = i;

				m_commandList->SetGraphicsRoot32BitConstants(1, 2, &rootData, 0);

				m_commandList->DrawInstanced(textureDim, textureDim, 0, 0);
			}
		}

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowReverseTransformBuffer->get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(voxelTextureColor->get()));

		m_commandList->Close();
		m_buildList = 1;
	}
}

void RenderPassInjectRSM::submitCommands(Graphics::CommandQueue* queue)
{
	if (m_buildList != 0)
	{
		queue->pushCommandList(&m_commandList);
		m_buildList = 0;
	}
}