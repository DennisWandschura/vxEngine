#include "RenderPassVisibleLights.h"
#include "d3dx12.h"
#include "GpuLight.h"
#include "ShaderManager.h"
#include "ResourceManager.h"
#include "CommandAllocator.h"
#include "CommandList.h"

RenderPassVisibleLights::RenderPassVisibleLights()
	:RenderPass(),
	m_lightCount(0),
	m_buildList(0)
{

}

RenderPassVisibleLights::~RenderPassVisibleLights()
{

}

void RenderPassVisibleLights::getRequiredMemory(u64* heapSizeBuffer, u32* bufferCount, u64* heapSizeTexture, u32* textureCount, u64* heapSizeRtDs, u32* rtDsCount, ID3D12Device* device)
{
}

bool RenderPassVisibleLights::createData(ID3D12Device* device)
{
	return true;
}

bool RenderPassVisibleLights::loadShaders()
{
	if (!s_shaderManager->loadShader(L"CopyLightsVS.cso"))
		return false;

	return true;
}

bool RenderPassVisibleLights::createRootSignature(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangeVS[2];
	rangeVS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0, 0);
	rangeVS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, 2);

	CD3DX12_ROOT_PARAMETER rootParameters[1];
	rootParameters[0].InitAsDescriptorTable(2, rangeVS, D3D12_SHADER_VISIBILITY_VERTEX);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(1, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	return m_rootSignature.create(&rootSignatureDesc, device);
}

bool RenderPassVisibleLights::createPipelineState(ID3D12Device* device)
{
	/*auto vsShader = s_shaderManager->getShader("CopyLightsVS.cso");

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { nullptr, 0 };
	psoDesc.pRootSignature = m_rootSignature.get();
	psoDesc.VS = { reinterpret_cast<UINT8*>(vsShader->GetBufferPointer()), vsShader->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.FrontCounterClockwise = 1;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = 0;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	psoDesc.NumRenderTargets = 0;
	psoDesc.SampleDesc.Count = 1;

	auto hresult = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipelineState.getAddressOf()));
	if (hresult != 0)
		return false;*/

	d3d::PipelineStateDescInput inputDesc;
	inputDesc.rootSignature = m_rootSignature.get();
	inputDesc.depthEnabled = 0;
	inputDesc.shaderDesc.vs = s_shaderManager->getShader(L"CopyLightsVS.cso");
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);

	return d3d::PipelineState::create(desc, &m_pipelineState, device);
}

bool RenderPassVisibleLights::initialize(ID3D12Device* device, d3d::CommandAllocator* allocators, u32 frameCount)
{
	if (!loadShaders())
		return false;

	if (!createRootSignature(device))
		return false;

	if (!createPipelineState(device))
		return false;

	if(!createCommandLists(device, D3D12_COMMAND_LIST_TYPE_DIRECT, allocators, frameCount))
		return false;

	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 3;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (!m_descriptorHeap.create(desc, device))
		return false;

	auto lightBuffer = s_resourceManager->getBuffer(L"lightBuffer");
	auto lightBufferDst = s_resourceManager->getBuffer(L"lightBufferDst");
	auto visibleLightIndexBuffer = s_resourceManager->getBuffer(L"visibleLightIndexBuffer");

	auto lightCount = s_settings->m_gpuLightCount + 1;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = lightCount;
	srvDesc.Buffer.StructureByteStride = sizeof(GpuLight);

	auto handle = m_descriptorHeap.getHandleCpu();
	device->CreateShaderResourceView(lightBuffer->get(), &srvDesc, handle);

	srvDesc.Buffer.StructureByteStride = sizeof(u32);
	handle.offset(1);
	device->CreateShaderResourceView(visibleLightIndexBuffer->get(), &srvDesc, handle);

	const auto sizeInBytes = lightCount * sizeof(GpuLight);
	//const auto count = sizeInBytes / sizeof(u32);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	uavDesc.Buffer.NumElements = lightCount;
	uavDesc.Buffer.StructureByteStride = sizeof(GpuLight);

	handle.offset(1);
	device->CreateUnorderedAccessView(lightBufferDst->get(), nullptr, &uavDesc, handle);

	return true;
}

void RenderPassVisibleLights::shutdown()
{
	m_descriptorHeap.destroy();
}

void RenderPassVisibleLights::buildCommands(d3d::CommandAllocator* currentAllocator, u32 frameIndex)
{
	if (m_lightCount != 0)
	{
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

		// ps res
		auto lightBuffer = s_resourceManager->getBuffer(L"lightBuffer");
		// ps res
		auto visibleLightIndexBuffer = s_resourceManager->getBuffer(L"visibleLightIndexBuffer");

		commandList->Reset(currentAllocator->get(), m_pipelineState.get());

		auto resolution = s_resolution;

		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &rectScissor);

		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(lightBuffer->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(visibleLightIndexBuffer->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

		auto descHeap = m_descriptorHeap.get();
		commandList->SetDescriptorHeaps(1, &descHeap);

		commandList->SetGraphicsRootSignature(m_rootSignature.get());
		commandList->SetGraphicsRootDescriptorTable(0, m_descriptorHeap->GetGPUDescriptorHandleForHeapStart());

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		commandList->DrawInstanced(m_lightCount + 1, 1, 0, 0);

		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(visibleLightIndexBuffer->get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(lightBuffer->get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		commandList->Close();
		m_buildList = 1;
	}
}

void RenderPassVisibleLights::submitCommands(Graphics::CommandQueue* queue)
{
	if (m_buildList != 0)
	{
		queue->pushCommandList(m_currentCommandList);
		m_buildList = 0;
	}
}