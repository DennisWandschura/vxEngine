#include "RenderPassSSIL.h"
#include "d3dx12.h"
#include "ShaderManager.h"
#include "ResourceManager.h"
#include "CommandAllocator.h"

RenderPassSSIL::RenderPassSSIL(d3d::CommandAllocator* cmdAlloc)
	:m_commandList(),
	m_cmdAlloc(cmdAlloc)
{

}

RenderPassSSIL::~RenderPassSSIL()
{

}


void RenderPassSSIL::getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device)
{
	D3D12_RESOURCE_DESC resDesc;
	resDesc.Alignment = 64 KBYTE;
	resDesc.DepthOrArraySize = 1;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	resDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	resDesc.Height = s_resolution.y;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Width = s_resolution.x;

	auto alloc = device->GetResourceAllocationInfo(1, 1, &resDesc);

	*heapSizeRtDs += alloc.SizeInBytes;
}

bool RenderPassSSIL::createData(ID3D12Device* device)
{
	return createTexture(device);
}

bool RenderPassSSIL::createTexture(ID3D12Device* device)
{
	D3D12_RESOURCE_DESC resDesc;
	resDesc.Alignment = 64 KBYTE;
	resDesc.DepthOrArraySize = 1;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	resDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	resDesc.Height = s_resolution.y;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Width = s_resolution.x;

	auto alloc = device->GetResourceAllocationInfo(1, 1, &resDesc);

	D3D12_CLEAR_VALUE clearValues[1];
	clearValues[0].Color[0] = 0.0f;
	clearValues[0].Color[1] = 0.0f;
	clearValues[0].Color[2] = 0.0f;
	clearValues[0].Color[3] = 0.0f;
	clearValues[0].Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

	CreateResourceDesc desc;
	desc.clearValue = clearValues;
	desc.resDesc = &resDesc;
	desc.size = alloc.SizeInBytes;
	desc.state = D3D12_RESOURCE_STATE_RENDER_TARGET;
	if (!s_resourceManager->createTextureRtDs(L"indirectLightTexture", desc))
		return false;

	return true;
}

bool RenderPassSSIL::loadShaders()
{
	if (!s_shaderManager->loadShader(L"DrawQuadVs.cso"))
		return false;

	if (!s_shaderManager->loadShader(L"DrawQuadGs.cso"))
		return false;

	if (!s_shaderManager->loadShader(L"SsilPS.cso"))
		return false;

	return true;
}

bool RenderPassSSIL::createRootSignature(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangePS[2];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 0);
	rangePS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 0, 1);

	CD3DX12_ROOT_PARAMETER rootParameters[1];
	rootParameters[0].InitAsDescriptorTable(2, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(1, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	return m_rootSignature.create(&rootSignatureDesc, device);
}

bool RenderPassSSIL::createPipelineState(ID3D12Device* device)
{
	auto rtvFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

	d3d::PipelineStateDescInput inputDesc;
	inputDesc.rootSignature = m_rootSignature.get();
	inputDesc.depthEnabled = 0;
	inputDesc.shaderDesc.vs = s_shaderManager->getShader(L"DrawQuadVs.cso");
	inputDesc.shaderDesc.gs = s_shaderManager->getShader(L"DrawQuadGs.cso");
	inputDesc.shaderDesc.ps = s_shaderManager->getShader(L"SsilPS.cso");
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	inputDesc.rtvFormats = &rtvFormat;
	inputDesc.rtvCount = 1;
	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);

	return d3d::PipelineState::create(desc, &m_pipelineState, device);
}

bool RenderPassSSIL::createSrv(ID3D12Device* device)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 7;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (!m_heapSrv.create(desc, device))
		return false;

	auto cameraStaticBufferView = s_resourceManager->getConstantBufferView("cameraStaticBufferView");

	auto handle = m_heapSrv.getHandleCpu();
	device->CreateConstantBufferView(cameraStaticBufferView, handle);

	auto zBuffer = s_resourceManager->getTextureRtDs(L"zBuffer0");
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescZBuffer;
	srvDescZBuffer.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescZBuffer.Format = DXGI_FORMAT_R32_FLOAT;
	srvDescZBuffer.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDescZBuffer.Texture2D.MipLevels = 1;
	srvDescZBuffer.Texture2D.MostDetailedMip = 0;
	srvDescZBuffer.Texture2D.PlaneSlice = 0;
	srvDescZBuffer.Texture2D.ResourceMinLODClamp = 0;

	handle.offset(1);
	device->CreateShaderResourceView(zBuffer->get(), &srvDescZBuffer, handle);

	auto directLightTexture = s_resourceManager->getTextureRtDs(L"directLightTexture");
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescDirect;
	srvDescDirect.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescDirect.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvDescDirect.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDescDirect.Texture2D.MipLevels = 1;
	srvDescDirect.Texture2D.MostDetailedMip = 0;
	srvDescDirect.Texture2D.PlaneSlice = 0;
	srvDescDirect.Texture2D.ResourceMinLODClamp = 0;
	handle.offset(1);
	device->CreateShaderResourceView(directLightTexture->get(), &srvDescDirect, handle);

	auto gbufferNormal = s_resourceManager->getTextureRtDs(L"gbufferNormal");
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescNormal;
	srvDescNormal.Format = DXGI_FORMAT_R16G16_FLOAT;
	srvDescNormal.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescNormal.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDescNormal.Texture2D.MipLevels = 1;
	srvDescNormal.Texture2D.MostDetailedMip = 0;
	srvDescNormal.Texture2D.PlaneSlice = 0;
	srvDescNormal.Texture2D.ResourceMinLODClamp = 0;

	handle.offset(1);
	device->CreateShaderResourceView(gbufferNormal->get(), &srvDescNormal, handle);

	return true;
}

bool RenderPassSSIL::createRtv(ID3D12Device* device)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	if (!m_heapRtv.create(desc, device))
		return false;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	auto indirectLightTexture = s_resourceManager->getTextureRtDs(L"indirectLightTexture");
	device->CreateRenderTargetView(indirectLightTexture->get(), &rtvDesc, m_heapRtv.getHandleCpu());

	return true;
}

bool RenderPassSSIL::initialize(ID3D12Device* device, void* p)
{
	if (!loadShaders())
		return false;

	if (!createRootSignature(device))
		return false;

	if (!createPipelineState(device))
		return false;

	if (!createSrv(device))
		return false;

	if (!createRtv(device))
		return false;

	if (!m_commandList.create(device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAlloc->get(), m_pipelineState.get()))
		return false;

	return true;
}

void RenderPassSSIL::shutdown()
{

}

void RenderPassSSIL::submitCommands(Graphics::CommandQueue* queue)
{
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

	auto directLightTexture = s_resourceManager->getTextureRtDs(L"directLightTexture");
	auto gbufferNormal = s_resourceManager->getTextureRtDs(L"gbufferNormal");
	auto zBuffer = s_resourceManager->getTextureRtDs(L"zBuffer0");

	const f32 clearColor[4] = { 0, 0, 0, 0 };
	m_commandList->Reset(m_cmdAlloc->get(), m_pipelineState.get());

	m_commandList->RSSetViewports(1, &viewport);
	m_commandList->RSSetScissorRects(1, &rectScissor);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(zBuffer->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gbufferNormal->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(directLightTexture->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	auto srvHeap = m_heapSrv.get();
	m_commandList->SetDescriptorHeaps(1, &srvHeap);

	m_commandList->SetGraphicsRootSignature(m_rootSignature.get());
	m_commandList->SetGraphicsRootDescriptorTable(0, srvHeap->GetGPUDescriptorHandleForHeapStart());

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_heapRtv.getHandleCpu();
	m_commandList->OMSetRenderTargets(1, &rtvHandle, 0, nullptr);
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	m_commandList->DrawInstanced(1, 1, 0, 0);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(directLightTexture->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gbufferNormal->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(zBuffer->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));

	auto hr = m_commandList->Close();

	queue->pushCommandList(&m_commandList);

	/*if (m_lightCount != 0)
	{
		auto albedoSlice = s_resourceManager->getTextureRtDs(L"gbufferAlbedo");
		auto gbufferDepth = s_resourceManager->getTextureRtDs(L"gbufferDepth");
		auto zBuffer = s_resourceManager->getTextureRtDs(L"zBuffer0");
		//auto lightBuffer = s_resourceManager->getBuffer(L"lightBuffer");
		auto gbufferSurface = s_resourceManager->getTextureRtDs(L"gbufferSurface");
		auto gbufferNormal = s_resourceManager->getTextureRtDs(L"gbufferNormal");
		//auto shadowTextureLinear = s_resourceManager->getTextureRtDs(L"shadowTextureLinear");
		//auto shadowTextureIntensity = s_resourceManager->getTextureRtDs(L"shadowTextureIntensity");

		const f32 clearColor[4] = { 0, 0, 0, 0 };
		m_commandList->Reset(m_cmdAlloc->get(), m_pipelineState.get());

		auto resolution = s_resolution;

	

		m_commandList->RSSetViewports(1, &viewport);
		m_commandList->RSSetScissorRects(1, &rectScissor);

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(albedoSlice->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gbufferDepth->get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(zBuffer->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gbufferNormal->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gbufferSurface->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		auto srvHeap = m_heapSrv.get();
		m_commandList->SetDescriptorHeaps(1, &srvHeap);

		m_commandList->SetGraphicsRootSignature(m_rootSignature.get());
		m_commandList->SetGraphicsRootDescriptorTable(0, srvHeap->GetGPUDescriptorHandleForHeapStart());
		m_commandList->SetGraphicsRootDescriptorTable(1, srvHeap->GetGPUDescriptorHandleForHeapStart());
		m_commandList->SetGraphicsRootDescriptorTable(2, srvHeap->GetGPUDescriptorHandleForHeapStart());

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_heapRtv.getHandleCpu();
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_heapDsv.getHandleCpu();
		m_commandList->OMSetRenderTargets(1, &rtvHandle, 0, &dsvHandle);
		m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		m_commandList->DrawInstanced(m_lightCount, 1, 0, 0);

		//m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowTextureIntensity->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
		//m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowTextureLinear->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gbufferNormal->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gbufferSurface->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(zBuffer->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(albedoSlice->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gbufferDepth->get(), D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));

		auto hr = m_commandList->Close();

		queue->pushCommandList(&m_commandList);
	}*/
}