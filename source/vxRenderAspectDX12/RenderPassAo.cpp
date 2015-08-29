#include "RenderPassAO.h"
#include "d3dx12.h"
#include "ShaderManager.h"
#include "ResourceManager.h"
#include "GpuSaoBuffer.h"
#include <vxLib/math/matrix.h>
#include "UploadManager.h"

RenderPassAO::RenderPassAO(const vx::uint2 &resolution, ID3D12CommandAllocator* cmdAlloc, f32 fov)
	:RenderPass(),
	m_commandList(),
	m_cmdAlloc(cmdAlloc),
	m_resolution(resolution),
	m_fov(fov)
{
}

RenderPassAO::~RenderPassAO()
{

}

void RenderPassAO::getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs, ID3D12Device* device)
{
	D3D12_RESOURCE_DESC resDesc;
	resDesc.Alignment = 64 KBYTE;
	resDesc.DepthOrArraySize = 1;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Height = m_resolution.y;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Width = m_resolution.x;

	auto allocInfo = device->GetResourceAllocationInfo(1, 1, &resDesc);

	*heapSizeBuffer += d3d::AlignedSizeType<GpuSaoBuffer, 1, 64 KBYTE>::size;
	*heapSizeRtDs += allocInfo.SizeInBytes * 2;
}

bool RenderPassAO::loadShaders(d3d::ShaderManager* shaderManager)
{
	if (!shaderManager->loadShader("DrawQuadVs.cso", L"../../lib/DrawQuadVs.cso", d3d::ShaderType::Vertex))
		return false;

	if (!shaderManager->loadShader("DrawQuadGs.cso", L"../../lib/DrawQuadGs.cso", d3d::ShaderType::Geometry))
		return false;

	if (!shaderManager->loadShader("SAO_aoPS.cso", L"../../lib/SAO_aoPS.cso", d3d::ShaderType::Pixel))
		return false;

	if (!shaderManager->loadShader("SAO_blurX.cso", L"../../lib/SAO_blurX.cso", d3d::ShaderType::Pixel))
		return false;

	if (!shaderManager->loadShader("SAO_blurY.cso", L"../../lib/SAO_blurY.cso", d3d::ShaderType::Pixel))
		return false;

	return true;
}

bool RenderPassAO::createRootSignature(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangePS[2];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, 0);
	rangePS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 1);

	CD3DX12_ROOT_PARAMETER rootParameters[1];
	rootParameters[0].InitAsDescriptorTable(2, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(1, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

bool RenderPassAO::createRootSignatureBlurX(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangePS[1];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, 2);

	CD3DX12_ROOT_PARAMETER rootParameters[1];
	rootParameters[0].InitAsDescriptorTable(1, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(1, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob* blob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	auto hresult = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &errorBlob);
	if (hresult != 0)
		return false;

	hresult = device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(m_blurRootSignature[0].getAddressOf()));
	if (hresult != 0)
		return false;

	return true;
}

bool RenderPassAO::createRootSignatureBlurY(ID3D12Device* device)
{
	CD3DX12_DESCRIPTOR_RANGE rangePS[1];
	rangePS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, 3);

	CD3DX12_ROOT_PARAMETER rootParameters[1];
	rootParameters[0].InitAsDescriptorTable(1, rangePS, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(1, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob* blob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	auto hresult = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &errorBlob);
	if (hresult != 0)
		return false;

	hresult = device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(m_blurRootSignature[1].getAddressOf()));
	if (hresult != 0)
		return false;

	return true;
}

bool RenderPassAO::createPipelineState(ID3D12Device* device, d3d::ShaderManager* shaderManager)
{
	auto vsShader = shaderManager->getShader("DrawQuadVs.cso");
	auto gsShader = shaderManager->getShader("DrawQuadGs.cso");
	auto psShader = shaderManager->getShader("SAO_aoPS.cso");

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { nullptr, 0 };
	psoDesc.pRootSignature = m_rootSignature.get();
	psoDesc.VS = { reinterpret_cast<UINT8*>(vsShader->GetBufferPointer()), vsShader->GetBufferSize() };
	psoDesc.GS = { reinterpret_cast<UINT8*>(gsShader->GetBufferPointer()), gsShader->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<UINT8*>(psShader->GetBufferPointer()), psShader->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.FrontCounterClockwise = 1;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = 0;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;

	auto hresult = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipelineState.getAddressOf()));
	if (hresult != 0)
		return false;

	psShader = shaderManager->getShader("SAO_blurX.cso");
	psoDesc.PS = { reinterpret_cast<UINT8*>(psShader->GetBufferPointer()), psShader->GetBufferSize() };
	psoDesc.pRootSignature = m_blurRootSignature[0].get();

	hresult = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_blurState[0].getAddressOf()));
	if (hresult != 0)
		return false;

	psShader = shaderManager->getShader("SAO_blurY.cso");
	psoDesc.PS = { reinterpret_cast<UINT8*>(psShader->GetBufferPointer()), psShader->GetBufferSize() };
	psoDesc.pRootSignature = m_blurRootSignature[1].get();

	hresult = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_blurState[1].getAddressOf()));
	if (hresult != 0)
		return false;

	return true;
}

bool RenderPassAO::createBuffer()
{
	const auto bufferSize = d3d::AlignedSizeType<GpuSaoBuffer, 1, 64 KBYTE>::size;
	auto saoBuffer = s_resourceManager->createBuffer(L"saoBuffer", bufferSize, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	if (saoBuffer == nullptr)
		return false;

	/*
	(float(-2.0 / (width * P[0][0])),
	float(-2.0 / (height * P[1][1])),
	float((1.0 - (double)P[0][2]) / P[0][0]),
	float((1.0 + (double)P[1][2]) / P[1][1]));

	abs(camera.imagePlanePixelsPerMeter(rd->viewport()))

	m_projection.imagePlanePixelsPerMeter(viewport);

	const float scale = -2.0f * tan(m_fieldOfView * 0.5f);

	if (m_direction == FOVDirection::HORIZONTAL) {
	return viewport.width() / scale;
	} else {
	return viewport.height() / scale;
	}
	*/

	//vx::mat4d P;
	//getProjectUnitMatrix(viewport, m_fov, 0.1, 100.0, P);

	f64 aspectRatio = (f64)m_resolution.x / m_resolution.y;
	auto P = vx::MatrixPerspectiveFovRHDX((f64)m_fov, aspectRatio, 0.1, 100.0);

	vx::float4a projInfo;
	projInfo.x = float(-2.0 / (m_resolution.x * P.c[0].m256d_f64[0]));
	projInfo.y = float(-2.0 / (m_resolution.y * P.c[1].m256d_f64[1]));
	projInfo.z = float((1.0 - (double)P.c[0].m256d_f64[2]) / P.c[0].m256d_f64[0]);
	projInfo.w = float((1.0 + (double)P.c[1].m256d_f64[2]) / P.c[1].m256d_f64[1]);

	const float scale = -2.0f * tan(m_fov * 0.5f);

	GpuSaoBuffer bufferData;
	bufferData.projInfo = projInfo;
	bufferData.bias = 0.012f;
	bufferData.intensity = 1.0f;
	bufferData.projScale = std::abs((f32)m_resolution.x / scale);
	bufferData.radius = 1.0f;

	s_uploadManager->pushUploadBuffer((u8*)&bufferData, saoBuffer, 0, sizeof(GpuSaoBuffer), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	return true;
}

bool RenderPassAO::createRtv(ID3D12Device* device, ID3D12Resource* texture)
{
	auto aoBlurXTexture = s_resourceManager->getTextureRtDs(L"aoBlurXTexture");

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	heapDesc.NodeMask = 1;
	heapDesc.NumDescriptors = 2;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	if (!m_rtvHeap.create(heapDesc, device))
		return false;

	D3D12_RENDER_TARGET_VIEW_DESC desc;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipSlice = 0;
	desc.Texture2D.PlaneSlice = 0;

	auto handle = m_rtvHeap.getHandleCpu();
	device->CreateRenderTargetView(texture, &desc, handle);

	handle.offset(1);
	device->CreateRenderTargetView(aoBlurXTexture, &desc, handle);

	return true;
}

bool RenderPassAO::createSrv(ID3D12Device* device)
{
	auto zBuffer = s_resourceManager->getTextureRtDs(L"zBuffer");
	auto saoBuffer = s_resourceManager->getBuffer(L"saoBuffer");
	auto aoBlurXTexture = s_resourceManager->getTextureRtDs(L"aoBlurXTexture");
	auto aoTexture = s_resourceManager->getTextureRtDs(L"aoTexture");

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	heapDesc.NumDescriptors = 4;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 1;
	if (!m_srvHeap.create(heapDesc, device))
		return false;

	auto handle = m_srvHeap.getHandleCpu();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 6;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	device->CreateShaderResourceView(zBuffer, &srvDesc, handle);

	handle.offset(1);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = saoBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = d3d::AlignedSizeType<GpuSaoBuffer, 1, 256>::size;
	device->CreateConstantBufferView(&cbvDesc, handle);

	handle.offset(1);
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(aoTexture, &srvDesc, handle);

	handle.offset(1);
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	device->CreateShaderResourceView(aoBlurXTexture, &srvDesc, handle);

	return true;
}

bool RenderPassAO::initialize(ID3D12Device* device, void* p)
{
	if (!loadShaders(s_shaderManager))
	{
		return false;
	}

	if (!createRootSignature(device))
	{
		return false;
	}

	if (!createRootSignatureBlurX(device))
		return false;

	if (!createRootSignatureBlurY(device))
		return false;

	if (!createPipelineState(device, s_shaderManager))
	{
		return false;
	}

	if (!createBuffer())
		return false;

	D3D12_CLEAR_VALUE clearValues[1];
	clearValues[0].Color[0] = 1.0f;
	clearValues[0].Color[1] = 1.0f;
	clearValues[0].Color[2] = 1.0f;
	clearValues[0].Color[3] = 1.0f;
	clearValues[0].Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	D3D12_RESOURCE_DESC resDesc;
	resDesc.Alignment = 64 KBYTE;
	resDesc.DepthOrArraySize = 1;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Height = m_resolution.y;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Width = m_resolution.x;

	auto allocInfo = device->GetResourceAllocationInfo(1, 1, &resDesc);

	CreateResourceDesc desc;
	desc.clearValue = clearValues;
	desc.resDesc = &resDesc;
	desc.size = allocInfo.SizeInBytes;
	desc.state = D3D12_RESOURCE_STATE_RENDER_TARGET;
	auto aoTexture = s_resourceManager->createTextureRtDs(L"aoTexture", desc);
	if (aoTexture == nullptr)
	{
		return false;
	}

	auto aoBlurXTexture = s_resourceManager->createTextureRtDs(L"aoBlurXTexture", desc);
	if (aoBlurXTexture == nullptr)
	{
		return false;
	}

	if (!m_commandList.create(device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAlloc, m_pipelineState.get()))
		return false;

	if (!createRtv(device, aoTexture))
		return false;


	if (!createSrv(device))
		return false;

	return true;
}

void RenderPassAO::shutdown()
{

}

ID3D12CommandList* RenderPassAO::submitCommands()
{
	auto aoTexture = s_resourceManager->getTextureRtDs(L"aoTexture");
	auto aoBlurXTexture = s_resourceManager->getTextureRtDs(L"aoBlurXTexture");

	const f32 clearColor[] = { 1, 1, 1, 1 };
	//u32 width = m_resolution.x - COMPUTE_GUARD_BAND;
	//u32 height = m_resolution.y - COMPUTE_GUARD_BAND;

	//auto res = m_resolution - COMPUTE_GUARD_BAND * 2;

	// rd->setClip2D(Rect2D::xyxy(guardBandSize, guardBandSize, rd->viewport().width() - guardBandSize, rd->viewport().height() - guardBandSize));

	D3D12_VIEWPORT viewPort;
	viewPort.Width = (f32)m_resolution.x;
	viewPort.Height = (f32)m_resolution.y;
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.TopLeftX = (f32)0;
	viewPort.TopLeftY = (f32)0;

	D3D12_RECT rectScissor;
	rectScissor.left = 0;
	rectScissor.right = m_resolution.x;
	rectScissor.top = 0;
	rectScissor.bottom = m_resolution.y;

	auto hr = m_commandList->Reset(m_cmdAlloc, m_pipelineState.get());

	m_commandList->SetGraphicsRootSignature(m_rootSignature.get());

	auto srvHeap = m_srvHeap.get();
	m_commandList->SetDescriptorHeaps(1, &srvHeap);
	m_commandList->SetGraphicsRootDescriptorTable(0, srvHeap->GetGPUDescriptorHandleForHeapStart());

	m_commandList->RSSetScissorRects(1, &rectScissor);
	m_commandList->RSSetViewports(1, &viewPort);

	auto rtvHandle = m_rtvHeap.getHandleCpu();
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandleRawAO = rtvHandle;
	m_commandList->OMSetRenderTargets(1, &rtvHandleRawAO, 0, nullptr);
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	m_commandList->DrawInstanced(1, 1, 0, 0);

	{
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(aoTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		m_commandList->SetPipelineState(m_blurState[0].get());

		m_commandList->SetGraphicsRootSignature(m_blurRootSignature[0].get());

		rtvHandle.offset(1);

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandleBlurAO = rtvHandle;
		m_commandList->OMSetRenderTargets(1, &rtvHandleBlurAO, 0, nullptr);
		m_commandList->ClearRenderTargetView(rtvHandleBlurAO, clearColor, 0, nullptr);

		m_commandList->DrawInstanced(1, 1, 0, 0);

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(aoTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	}

	{
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(aoBlurXTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		m_commandList->SetPipelineState(m_blurState[1].get());

		m_commandList->SetGraphicsRootSignature(m_blurRootSignature[1].get());

		m_commandList->OMSetRenderTargets(1, &rtvHandleRawAO, 0, nullptr);

		m_commandList->DrawInstanced(1, 1, 0, 0);

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(aoBlurXTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	}

	m_commandList->Close();

	return m_commandList.get();
}