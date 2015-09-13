#include "RenderPassShading.h"
#include "d3dx12.h"
#include "ShaderManager.h"
#include "ResourceManager.h"
#include "GpuLight.h"
#include "GpuShadowTransform.h"
#include "CommandAllocator.h"

RenderPassShading::RenderPassShading(d3d::CommandAllocator* cmdAlloc)
	:m_commandList(),
	m_cmdAlloc(cmdAlloc),
	m_lightCount(0)
{

}

RenderPassShading::~RenderPassShading()
{

}


void RenderPassShading::getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device)
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

bool RenderPassShading::createData(ID3D12Device* device)
{
	return createTexture(device);
}

bool RenderPassShading::createTexture(ID3D12Device* device)
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
	if (!s_resourceManager->createTextureRtDs(L"directLightTexture", desc))
		return false;

	return true;
}

bool RenderPassShading::loadShaders()
{
	if (!s_shaderManager->loadShader("ShadeVS.cso", L"../../lib/ShadeVS.cso", d3d::ShaderType::Vertex))
		return false;

	if (!s_shaderManager->loadShader("ShadeGS.cso", L"../../lib/ShadeGS.cso", d3d::ShaderType::Geometry))
		return false;

	if (!s_shaderManager->loadShader("ShadePS.cso", L"../../lib/ShadePS.cso", d3d::ShaderType::Pixel))
		return false;

	return true;
}

bool RenderPassShading::createRootSignature(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangeVS[2];
	rangeVS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 0);
	rangeVS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, 2);

	CD3DX12_DESCRIPTOR_RANGE rangeGS[1];
	rangeGS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 0);

	CD3DX12_DESCRIPTOR_RANGE rangePS[2];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0, 0, 0);
	rangePS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 1, 0, 3);

	CD3DX12_ROOT_PARAMETER rootParameters[3];
	rootParameters[0].InitAsDescriptorTable(2, rangeVS, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsDescriptorTable(1, rangeGS, D3D12_SHADER_VISIBILITY_GEOMETRY);
	rootParameters[2].InitAsDescriptorTable(2, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);

	D3D12_STATIC_SAMPLER_DESC sampler[2];
	sampler[0].Filter = D3D12_FILTER_ANISOTROPIC;
	sampler[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler[0].MipLODBias = 0;
	sampler[0].MaxAnisotropy = 4;
	sampler[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler[0].MinLOD = 0.0f;
	sampler[0].MaxLOD = 0.0f;
	sampler[0].ShaderRegister = 0;
	sampler[0].RegisterSpace = 0;
	sampler[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	sampler[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler[1].MipLODBias = 0;
	sampler[1].MaxAnisotropy = 0;
	sampler[1].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS;
	sampler[1].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	sampler[1].MinLOD = 0.0f;
	sampler[1].MaxLOD = 0.0f;
	sampler[1].ShaderRegister = 1;
	sampler[1].RegisterSpace = 0;
	sampler[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(3, rootParameters, 2, sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

bool RenderPassShading::createPipelineState(ID3D12Device* device)
{
	auto rtvFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

	d3d::PipelineStateDescInput inputDesc;
	inputDesc.rootSignature = m_rootSignature.get();
	inputDesc.depthEnabled = 1;
	inputDesc.shaderDesc.vs = s_shaderManager->getShader("ShadeVS.cso");
	inputDesc.shaderDesc.gs = s_shaderManager->getShader("ShadeGS.cso");
	inputDesc.shaderDesc.ps = s_shaderManager->getShader("ShadePS.cso");
	inputDesc.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	inputDesc.rtvFormats = &rtvFormat;
	inputDesc.rtvCount = 1;
	inputDesc.dsvFormat = DXGI_FORMAT_D32_FLOAT;
	auto desc = d3d::PipelineState::getDefaultDescription(inputDesc);

	desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
	desc.BlendState.RenderTarget[0].BlendEnable = 1;
	desc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	desc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	desc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;

	return d3d::PipelineState::create(desc, &m_pipelineState, device);
}

bool RenderPassShading::createSrv(ID3D12Device* device)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 8;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (!m_heapSrv.create(desc, device))
		return false;

	/*
	cbuffer CameraBuffer : register(b0)
	cbuffer CameraStaticBuffer : register(b1)
	StructuredBuffer<GpuLight> g_lights : register(t0);
	Texture2DArray g_albedoeSlice : register(t1);
	Texture2DArray g_normalSlice : register(t2);
	Texture2D<float> g_zBuffer : register(t3);
	Texture2DArray g_surfaceSlice : register(t4);
	TextureCube<float> g_shadowTexture : register(t5);
	*/

	auto cameraBufferView = s_resourceManager->getConstantBufferView("cameraBufferView");
	auto cameraStaticBufferView = s_resourceManager->getConstantBufferView("cameraStaticBufferView");

	auto handle = m_heapSrv.getHandleCpu();
	device->CreateConstantBufferView(cameraBufferView, handle);

	handle.offset(1);
	device->CreateConstantBufferView(cameraStaticBufferView, handle);

	auto shadowCastingLightsBuffer = s_resourceManager->getBuffer(L"shadowCastingLightsBuffer");
	auto shadowCastingLightsBufferView = s_resourceManager->getShaderResourceView("shadowCastingLightsBufferView");

	handle.offset(1);
	device->CreateShaderResourceView(shadowCastingLightsBuffer->get(), shadowCastingLightsBufferView, handle);

	auto albedoSlice = s_resourceManager->getTextureRtDs(L"gbufferAlbedo");

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.ArraySize = 1;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.MipLevels = 1;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.PlaneSlice = 0;
	srvDesc.Texture2DArray.ResourceMinLODClamp = 0;

	handle.offset(1);
	device->CreateShaderResourceView(albedoSlice->get(), &srvDesc, handle);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescZBuffer;
	srvDescZBuffer.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescZBuffer.Format = DXGI_FORMAT_R32_FLOAT;
	srvDescZBuffer.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDescZBuffer.Texture2D.MipLevels = 1;
	srvDescZBuffer.Texture2D.MostDetailedMip = 0;
	srvDescZBuffer.Texture2D.PlaneSlice = 0;
	srvDescZBuffer.Texture2D.ResourceMinLODClamp = 0;
	handle.offset(1);
	auto zBuffer = s_resourceManager->getTextureRtDs(L"zBuffer0");
	device->CreateShaderResourceView(zBuffer->get(), &srvDescZBuffer, handle);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescNormal;
	srvDescNormal.Format = DXGI_FORMAT_R16G16_FLOAT;
	srvDescNormal.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescNormal.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDescNormal.Texture2DArray.ArraySize = 1;
	srvDescNormal.Texture2DArray.FirstArraySlice = 0;
	srvDescNormal.Texture2DArray.MipLevels = 1;
	srvDescNormal.Texture2DArray.MostDetailedMip = 0;
	srvDescNormal.Texture2DArray.PlaneSlice = 0;
	srvDescNormal.Texture2DArray.ResourceMinLODClamp = 0;

	auto gbufferNormal = s_resourceManager->getTextureRtDs(L"gbufferNormal");
	handle.offset(1);
	device->CreateShaderResourceView(gbufferNormal->get(), &srvDescNormal, handle);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescSurface;
	srvDescSurface.Format = DXGI_FORMAT_R16G16_FLOAT;
	srvDescSurface.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescSurface.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDescSurface.Texture2DArray.ArraySize = 1;
	srvDescSurface.Texture2DArray.FirstArraySlice = 0;
	srvDescSurface.Texture2DArray.MipLevels = 1;
	srvDescSurface.Texture2DArray.MostDetailedMip = 0;
	srvDescSurface.Texture2DArray.PlaneSlice = 0;
	srvDescSurface.Texture2DArray.ResourceMinLODClamp = 0;
	handle.offset(1);
	auto gbufferSurface = s_resourceManager->getTextureRtDs(L"gbufferSurface");
	device->CreateShaderResourceView(gbufferSurface->get(), &srvDescSurface, handle);

	auto shadowTextureLinear = s_resourceManager->getTextureRtDs(L"shadowTextureLinear");
	srvDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
	srvDesc.TextureCubeArray.First2DArrayFace = 0;
	srvDesc.TextureCubeArray.MipLevels = 1;
	srvDesc.TextureCubeArray.MostDetailedMip = 0;
	srvDesc.TextureCubeArray.NumCubes = 10;
	srvDesc.TextureCubeArray.ResourceMinLODClamp = 0;
	handle.offset(1);
	device->CreateShaderResourceView(shadowTextureLinear->get(), &srvDesc, handle);

	return true;
}

bool RenderPassShading::createRtv(ID3D12Device* device)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	if (!m_heapRtv.create(desc, device))
		return false;

	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	if (!m_heapDsv.create(desc, device))
		return false;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	auto directLightTexture = s_resourceManager->getTextureRtDs(L"directLightTexture");
	device->CreateRenderTargetView(directLightTexture->get(), &rtvDesc, m_heapRtv.getHandleCpu());

	auto gbufferDepth = s_resourceManager->getTextureRtDs(L"gbufferDepth");

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
	dsvDesc.Texture2DArray.ArraySize = 1;
	dsvDesc.Texture2DArray.FirstArraySlice = 0;
	dsvDesc.Texture2DArray.MipSlice = 0;
	device->CreateDepthStencilView(gbufferDepth->get(), &dsvDesc, m_heapDsv.getHandleCpu());

	return true;
}

bool RenderPassShading::initialize(ID3D12Device* device, void* p)
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

void RenderPassShading::shutdown()
{

}

void RenderPassShading::submitCommands(Graphics::CommandQueue* queue)
{
	if (m_lightCount != 0)
	{
		auto albedoSlice = s_resourceManager->getTextureRtDs(L"gbufferAlbedo");
		auto gbufferDepth = s_resourceManager->getTextureRtDs(L"gbufferDepth");
		auto zBuffer = s_resourceManager->getTextureRtDs(L"zBuffer0");
		auto gbufferSurface = s_resourceManager->getTextureRtDs(L"gbufferSurface");
		auto gbufferNormal = s_resourceManager->getTextureRtDs(L"gbufferNormal");
		auto shadowTextureLinear = s_resourceManager->getTextureRtDs(L"shadowTextureLinear");
		auto shadowCastingLightsBuffer = s_resourceManager->getBuffer(L"shadowCastingLightsBuffer");

		const f32 clearColor[4] = { 0, 0, 0, 0 };
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

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(albedoSlice->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gbufferDepth->get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(zBuffer->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gbufferNormal->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gbufferSurface->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowTextureLinear->get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowCastingLightsBuffer->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

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

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowCastingLightsBuffer->get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowTextureLinear->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gbufferNormal->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gbufferSurface->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(zBuffer->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(albedoSlice->get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gbufferDepth->get(), D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));

		auto hr = m_commandList->Close();

		queue->pushCommandList(&m_commandList);
	}
}