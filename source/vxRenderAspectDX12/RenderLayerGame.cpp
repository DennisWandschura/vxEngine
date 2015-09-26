#include "RenderLayerGame.h"
#include <vxEngineLib/Message.h>
#include <vxEngineLib/FileMessage.h>
#include <vxEngineLib/MessageTypes.h>
#include <vxEngineLib/Scene.h>
#include <vxLib/Graphics/Camera.h>
#include <vxEngineLib/MeshInstance.h>
#include <vxEngineLib/Graphics/RenderUpdateTask.h>
#include "MaterialManager.h"
#include <vxEngineLib/RendererMessage.h>
#include "ResourceManager.h"
#include <vxEngineLib/CreateActorData.h>
#include <vxEngineLib/ResourceAspectInterface.h>
#include "UploadManager.h"
#include "d3dx12.h"
#include <vxEngineLib/IngameMessage.h>
#include <vxEngineLib/MessageManager.h>
#include <vxEngineLib/GpuFunctions.h>
#include <vxEngineLib/MeshFile.h>
#include "RenderPassGBuffer.h"
#include "RenderPassShadow.h"
#include "RenderPassZBuffer.h"
#include "RenderPassZBufferCreateMipmaps.h"
#include "RenderPassAo.h"
#include "RenderPassShading.h"
#include "RenderPassFinal.h"
#include <vxEngineLib/CreateDynamicMeshData.h>
#include "Device.h"
#include "CopyManager.h"
#include "RenderPassCullLights.h"
#include "RenderPassSSIL.h"
#include "RenderPassFilterRSM.h"
#include "RenderPassInjectRSM.h"
#include "RenderPassDrawVoxel.h"
#include "RenderPassVoxelize.h"
#include "RenderPassVoxelMip.h"
#include "RenderPassOcclusion.h"
#include "RenderPassConeTrace.h"
#include "GpuVoxel.h"
#include "RenderPassVoxelPropagate.h"
#include <vxEngineLib/CpuProfiler.h>

const u32 g_swapChainBufferCount{ 2 };
const u32 g_maxVertexCount{ 20000 };
const u32 g_maxIndexCount{ 40000 };
const u32 g_maxMeshInstances{ 128 };

namespace RenderLayerGameCpp
{
	D3D12_RESOURCE_DESC getResDescVoxelOpacity(u32 dim)
	{
		D3D12_RESOURCE_DESC desc;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		desc.Alignment = 64 KBYTE;
		desc.Width = dim;
		desc.Height = dim;
		desc.DepthOrArraySize = dim;
		desc.MipLevels = 4;
		desc.Format = DXGI_FORMAT_R32_UINT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		return desc;
	}

	D3D12_RESOURCE_DESC getResDescVoxelColor(u32 dim, u32 mipLevels)
	{
		D3D12_RESOURCE_DESC desc;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		desc.Alignment = 64 KBYTE;
		desc.Width = dim;
		desc.Height = dim;
		desc.DepthOrArraySize = dim * 6;
		desc.MipLevels = mipLevels;
		desc.Format = DXGI_FORMAT_R8G8B8A8_TYPELESS;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		return desc;
	}

	D3D12_RESOURCE_DESC getResDescVoxelColorTemp(u32 dim)
	{
		D3D12_RESOURCE_DESC desc;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		desc.Alignment = 64 KBYTE;
		desc.Width = dim;
		desc.Height = dim;
		desc.DepthOrArraySize = dim * 6;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_TYPELESS;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		return desc;
	}
}

RenderLayerGame::RenderLayerGame(const RenderLayerGameDesc &desc)
	:m_renderPasses(),
	m_copyManager(desc.m_copyManager),
	m_commandAllocator(),
	m_lightManager(),
	m_camera(desc.m_camera),
	m_frustum(desc.m_frustum),
	m_drawCommandMesh(),
	m_cpuProfiler(desc.m_cpuProfiler),
	m_device(desc.m_device),
	m_uploadManager(desc.m_uploadManager),
	m_msgManager(desc.m_msgManager),
	m_resourceManager(desc.m_resourceManager),
	m_materialManager(desc.m_materialManager),
	m_resourceAspect(desc.m_resourceAspect),
	m_meshManager(),
	m_downloadManager(desc.m_downloadManager),
	m_settings(desc.m_settings)
{
}

RenderLayerGame::RenderLayerGame(RenderLayerGame &&rhs)
	:m_renderPasses(std::move(rhs.m_renderPasses)),
	m_copyManager(rhs.m_copyManager),
	m_commandAllocator(std::move(rhs.m_commandAllocator)),
	m_lightManager(std::move(rhs.m_lightManager)),
	m_camera(rhs.m_camera),
	m_frustum(rhs.m_frustum),
	m_lastVoxelCenter(0, 0, 0),
	m_drawCommandMesh(std::move(m_drawCommandMesh)),
	m_cpuProfiler(rhs.m_cpuProfiler),
	m_device(rhs.m_device),
	m_uploadManager(rhs.m_uploadManager),
	m_msgManager(rhs.m_msgManager),
	m_resourceManager(rhs.m_resourceManager),
	m_materialManager(rhs.m_materialManager),
	m_resourceAspect(rhs.m_resourceAspect),
	m_meshManager(std::move(rhs.m_meshManager)),
	m_downloadManager(rhs.m_downloadManager),
	m_settings(rhs.m_settings)
{

}

RenderLayerGame::~RenderLayerGame()
{

}

void RenderLayerGame::createRenderPasses()
{
	insertRenderPass(vx::make_sid("RenderPassGBuffer"), std::make_unique<RenderPassGBuffer>(&m_commandAllocator, &m_drawCommandMesh));
	insertRenderPass(vx::make_sid("RenderPassCullLights"), std::make_unique<RenderPassCullLights>(&m_commandAllocator, m_downloadManager));
	insertRenderPass(vx::make_sid("RenderPassShadow"), std::make_unique<RenderPassShadow>(&m_commandAllocator, &m_drawCommandMesh));
	insertRenderPass(vx::make_sid("RenderPassVoxelize"), std::make_unique<RenderPassVoxelize>(&m_commandAllocator, &m_drawCommandMesh));
	insertRenderPass(vx::make_sid("RenderPassInjectRSM"), std::make_unique<RenderPassInjectRSM>(&m_commandAllocator));
	insertRenderPass(vx::make_sid("RenderPassVoxelPropagate"), std::make_unique<RenderPassVoxelPropagate>(&m_commandAllocator));
	insertRenderPass(vx::make_sid("RenderPassZBuffer"), std::make_unique<RenderPassZBuffer>(&m_commandAllocator));
	insertRenderPass(vx::make_sid("RenderPassZBufferCreateMipmaps"), std::make_unique<RenderPassZBufferCreateMipmaps>(&m_commandAllocator));
	insertRenderPass(vx::make_sid("RenderPassAO"), std::make_unique<RenderPassAO>(&m_commandAllocator));
	insertRenderPass(vx::make_sid("RenderPassConeTrace"), std::make_unique<RenderPassConeTrace>(&m_commandAllocator));
	insertRenderPass(vx::make_sid("RenderPassShading"), std::move(std::make_unique<RenderPassShading>(&m_commandAllocator)));
	insertRenderPass(vx::make_sid("RenderPassSSIL"), std::make_unique<RenderPassSSIL>(&m_commandAllocator));
	insertRenderPass(vx::make_sid("RenderPassFinal"), std::make_unique<RenderPassFinal>(&m_commandAllocator, m_device));
	insertRenderPass(vx::make_sid("RenderPassFilterRSM"), std::make_unique<RenderPassFilterRSM>(&m_commandAllocator));

	//m_lightManager.setRenderPassCullLights(renderPassCullLights.get());
	//pushRenderPass(std::move(renderPassCullLights));

	//auto rnederPassShadow = std::make_unique<RenderPassShadow>(&m_commandAllocator, &m_drawCommandMesh);
	//m_lightManager.addRenderPass(rnederPassShadow.get());
	//pushRenderPass(std::move(rnederPassShadow));

	//pushRenderPass(std::move(std::make_unique<RenderPassVoxelize>(&m_commandAllocator, &m_drawCommandMesh)));

	RenderStage stage0;
	stage0.pushRenderPass(findRenderPass("RenderPassGBuffer"));
	stage0.pushRenderPass(findRenderPass("RenderPassCullLights"));
	stage0.pushRenderPass(findRenderPass("RenderPassShadow"));
	stage0.pushRenderPass(findRenderPass("RenderPassVoxelize"));
	stage0.pushRenderPass(findRenderPass("RenderPassFilterRSM"));
	stage0.pushRenderPass(findRenderPass("RenderPassInjectRSM"));

	RenderStage stage1;
	stage0.pushRenderPass(findRenderPass("RenderPassVoxelPropagate"));

	RenderStage stage2;
	stage2.pushRenderPass(findRenderPass("RenderPassZBuffer"));
	stage2.pushRenderPass(findRenderPass("RenderPassZBufferCreateMipmaps"));
	stage2.pushRenderPass(findRenderPass("RenderPassAO"));
	stage2.pushRenderPass(findRenderPass("RenderPassConeTrace"));
	stage2.pushRenderPass(findRenderPass("RenderPassShading"));
	stage2.pushRenderPass(findRenderPass("RenderPassSSIL"));
	stage2.pushRenderPass(findRenderPass("RenderPassFinal"));

	m_renderStages.push_back(stage0);
	m_renderStages.push_back(stage1);
	m_renderStages.push_back(stage2);

	m_lightManager.setRenderPassCullLights((RenderPassCullLights*)findRenderPass("RenderPassCullLights"));
	m_lightManager.addRenderPass((RenderPassLight*)findRenderPass("RenderPassShadow"));
	m_lightManager.addRenderPass((RenderPassLight*)findRenderPass("RenderPassFilterRSM"));
	m_lightManager.addRenderPass((RenderPassLight*)findRenderPass("RenderPassInjectRSM"));
	m_lightManager.addRenderPass((RenderPassLight*)findRenderPass("RenderPassShading"));

	/*auto renderPassFilterRSM = std::make_unique<RenderPassFilterRSM>(&m_commandAllocator);
	m_lightManager.addRenderPass(renderPassFilterRSM.get());
	pushRenderPass(std::move(renderPassFilterRSM));

	auto renderPassInjectRSM = std::make_unique<RenderPassInjectRSM>(&m_commandAllocator);
	m_lightManager.addRenderPass(renderPassInjectRSM.get());
	pushRenderPass(std::move(renderPassInjectRSM));*/

	//pushRenderPass(std::move(std::make_unique<RenderPassVoxelPropagate>(&m_commandAllocator)));

	//pushRenderPass(std::make_unique<RenderPassZBuffer>(&m_commandAllocator));
	//pushRenderPass(std::make_unique<RenderPassZBufferCreateMipmaps>(&m_commandAllocator));
	//pushRenderPass(std::make_unique<RenderPassAO>(&m_commandAllocator));

	//pushRenderPass(std::make_unique<RenderPassVoxelMip>(&m_commandAllocator));

	/*pushRenderPass(std::make_unique<RenderPassConeTrace>(&m_commandAllocator));

	auto renderPassShading = std::make_unique<RenderPassShading>(&m_commandAllocator);
	m_lightManager.addRenderPass(renderPassShading.get());
	pushRenderPass(std::move(renderPassShading));

	pushRenderPass(std::make_unique<RenderPassBlurVoxel>(&m_commandAllocator));

	pushRenderPass(std::make_unique<RenderPassSSIL>(&m_commandAllocator));
	pushRenderPass(std::make_unique<RenderPassFinal>(&m_commandAllocator, m_device));*/
}

void RenderLayerGame::getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs)
{
	m_lightManager.getRequiredMemory(heapSizeBuffer, m_settings->m_gpuLightCount, m_settings->m_shadowCastingLightCount);

	m_drawCommandMesh.getRequiredMemory(g_maxMeshInstances, heapSizeBuffer);

	auto device = m_device->getDevice();
	for (auto &it : m_renderPasses)
	{
		it->getRequiredMemory(heapSizeBuffer, heapSizeTexture, heapSizeRtDs, device);
	}

	auto resDescVoxelOpacity = RenderLayerGameCpp::getResDescVoxelOpacity(m_settings->m_lpvDim);
	auto resDescVoxelColor = RenderLayerGameCpp::getResDescVoxelColor(m_settings->m_lpvDim, m_settings->m_lpvMip);
	auto voxelOpacityInfo = m_device->getDevice()->GetResourceAllocationInfo(1,1, &resDescVoxelOpacity);
	auto voxelColorInfo = m_device->getDevice()->GetResourceAllocationInfo(1, 1, &resDescVoxelColor);

	auto resDescVoxelColorLast = RenderLayerGameCpp::getResDescVoxelColorTemp(m_settings->m_lpvDim);
	auto allocInfoVoxelColorLast = m_device->getDevice()->GetResourceAllocationInfo(1, 1, &resDescVoxelColorLast);

	*heapSizeTexture += voxelOpacityInfo.SizeInBytes + voxelColorInfo.SizeInBytes + allocInfoVoxelColorLast.SizeInBytes;
}

void RenderLayerGame::createGpuObjects()
{
	auto resDescVoxelOpacity = RenderLayerGameCpp::getResDescVoxelOpacity(m_settings->m_lpvDim);
	auto resDescVoxelColor = RenderLayerGameCpp::getResDescVoxelColor(m_settings->m_lpvDim, m_settings->m_lpvMip);
	auto voxelOpacityInfo = m_device->getDevice()->GetResourceAllocationInfo(1, 1, &resDescVoxelOpacity);
	auto voxelColorInfo = m_device->getDevice()->GetResourceAllocationInfo(1, 1, &resDescVoxelColor);

	CreateResourceDesc lpvDesc;
	lpvDesc.clearValue = nullptr;
	lpvDesc.resDesc = &resDescVoxelOpacity;
	lpvDesc.size = voxelOpacityInfo.SizeInBytes;
	lpvDesc.state = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

	m_resourceManager->createTexture(L"voxelTextureOpacity", lpvDesc);

	lpvDesc.resDesc = &resDescVoxelColor;
	lpvDesc.size = voxelColorInfo.SizeInBytes;
	m_resourceManager->createTexture(L"voxelTextureColor", lpvDesc);

	auto resDescVoxelColorLast = RenderLayerGameCpp::getResDescVoxelColorTemp(m_settings->m_lpvDim);
	auto allocInfoVoxelColorLast = m_device->getDevice()->GetResourceAllocationInfo(1, 1, &resDescVoxelColorLast);
	lpvDesc.resDesc = &resDescVoxelColorLast;
	lpvDesc.size = allocInfoVoxelColorLast.SizeInBytes;
	m_resourceManager->createTexture(L"voxelTextureColorTmp", lpvDesc);
}

bool RenderLayerGame::initialize(vx::StackAllocator* allocator)
{
	auto device = m_device->getDevice();
	if (!m_commandAllocator.create(D3D12_COMMAND_LIST_TYPE_DIRECT, device))
	{
		return false;
	}

	if (!m_lightManager.initialize(*m_settings, allocator, m_settings->m_gpuLightCount, m_resourceManager, m_uploadManager))
		return false;

	if (!m_meshManager.initialize(g_maxVertexCount, g_maxIndexCount, g_maxMeshInstances, m_resourceManager, device, allocator))
		return false;

	if (!m_drawCommandMesh.create(L"drawCmdBuffer", g_maxMeshInstances, m_resourceManager, m_device->getDevice()))
		return false;

	createGpuObjects();

	for (auto &it : m_renderPasses)
	{
		if (!it->createData(device))
			return false;
	}

	for (auto &it : m_renderPasses)
	{
		if (!it->initialize(device, nullptr))
			return false;
	}

	auto gridsize = m_settings->m_lpvGridSize;
	auto dim = m_settings->m_lpvDim;
	auto halfDim = dim / 2;
	auto gridHalfSize = gridsize / 2.0f;
	auto gridCellSize = gridHalfSize / halfDim;
	auto invGridCellSize = 1.0f / gridCellSize;
	m_gridCellSize = gridCellSize;
	m_invGridCellSize = invGridCellSize;

	return true;
}

void RenderLayerGame::shudown()
{
	for (auto &it : m_renderPasses)
	{
		it->shutdown();
	}
	m_renderPasses.clear();

	m_drawCommandMesh.destroy();
	m_meshManager.shutdown();
	m_commandAllocator.destroy();

	m_copyManager = nullptr;
}

void RenderLayerGame::update()
{
	m_cpuProfiler->pushMarker("RenderLayerGame::update");

	auto camPos = m_camera->getPosition();
	auto camRot = m_camera->getRotation();
	__m128 cameraPosition = _mm256_cvtpd_ps(camPos);
	__m128 cameraRotation = _mm256_cvtpd_ps(camRot);

	__m128 cameraDirection = { 0, 0, -1, 0 };
	cameraDirection = vx::quaternionRotation(cameraDirection, cameraRotation);

	m_lightManager.update(cameraPosition, cameraDirection, *m_frustum, m_resourceManager, m_uploadManager);

	auto voxelBuffer = m_resourceManager->getBuffer(L"voxelBuffer");

	__m128 invGridCellSize = { m_invGridCellSize, m_invGridCellSize, m_invGridCellSize, m_invGridCellSize };
	__m128 gridCellSize = { m_gridCellSize, m_gridCellSize, m_gridCellSize, m_gridCellSize };

	vx::float4a voxelCenter = _mm_mul_ps(cameraPosition, invGridCellSize);
	voxelCenter = _mm_floor_ps(voxelCenter);
	voxelCenter = _mm_mul_ps(voxelCenter, gridCellSize);

	auto newVoxelCenter = vx::int3(voxelCenter.x, voxelCenter.y, voxelCenter.z);
	if (m_lastVoxelCenter.x != newVoxelCenter.x ||
		m_lastVoxelCenter.y != newVoxelCenter.y ||
		m_lastVoxelCenter.z != newVoxelCenter.z)
	{
		m_uploadManager->pushUploadBuffer(voxelCenter, voxelBuffer->get(), offsetof(GpuVoxel, gridCenter), voxelBuffer->getOriginalState());
		//m_uploadManager->pushUploadBuffer(vx::float4a(m_lastVoxelCenter.x, m_lastVoxelCenter.y, m_lastVoxelCenter.z, 1), voxelBuffer->get(), offsetof(GpuVoxel, prevGridCenter), voxelBuffer->getOriginalState());

		//auto voxelTextureColor = m_resourceManager->getTexture(L"voxelTextureColor");
		//auto voxelTextureColorPrevious = m_resourceManager->getTexture(L"voxelTextureColorPrevious");
		////m_copyManager->pushCopyTexture(voxelTextureColor->get(), voxelTextureColor->getOriginalState(), voxelTextureColorPrevious->get(), voxelTextureColorPrevious->getOriginalState());

		//m_lastVoxelCenter = newVoxelCenter;
	}

	m_cpuProfiler->popMarker();
}

void RenderLayerGame::buildCommandLists()
{
	auto hr = m_commandAllocator.reset();
	VX_ASSERT(hr == 0);
	for (auto &it : m_renderStages)
	{
		it.buildCommands();
	}
}

void RenderLayerGame::submitCommandLists(Graphics::CommandQueue* queue)
{
	/*for (auto &it : m_renderPasses)
	{
		it->submitCommands(queue);
	}*/
	for (auto &it : m_renderStages)
	{
		it.submitCommands(queue);
	}
	queue->execute();
}

u32 RenderLayerGame::getCommandListCount() const
{
	return m_renderPasses.size();
}

void RenderLayerGame::insertRenderPass(vx::StringID &&sid, std::unique_ptr<RenderPass> &&renderPass)
{
	m_renderPasses.insert(std::move(sid), std::move(renderPass));
}

void RenderLayerGame::insertRenderPass(const char* id, std::unique_ptr<RenderPass> &&renderPass)
{
	insertRenderPass(std::move(vx::make_sid(id)), std::move(renderPass));
}

RenderPass* RenderLayerGame::findRenderPass(const char* id)
{
	auto it = m_renderPasses.find(vx::make_sid(id));
	return (it == m_renderPasses.end()) ? nullptr : it->get();
}

void RenderLayerGame::handleMessage(const vx::Message &msg)
{
	switch (msg.type)
	{
	case(vx::MessageType::Renderer) :
		handleRendererMessage(msg);
			break;
	case(vx::MessageType::File) :
		handleFileMessage(msg);
		break;
	default:
		break;
	}
}

void RenderLayerGame::handleRendererMessage(const vx::Message &msg)
{
	auto type = (vx::RendererMessage)msg.code;

	switch (type)
	{
	case vx::RendererMessage::AddActor:
	{
		auto data = (CreateActorData*)msg.arg1.ptr;
		createActorGpuIndex(data);
	}break;
	case vx::RendererMessage::AddStaticMesh:
	{
		auto instancePtr = (MeshInstance*)msg.arg1.ptr;
		auto sid = vx::StringID(msg.arg2.u64);
		addStaticMeshInstance(*instancePtr, sid);
	}break;
	case vx::RendererMessage::AddDynamicMesh:
	{
		auto data = (CreateDynamicMeshData*)msg.arg1.ptr;
		addDynamicMeshInstance(data);
	}break;
	default:
		break;
	}
}

void RenderLayerGame::handleFileMessage(const vx::Message &msg)
{
	auto fileEvent = (vx::FileMessage)msg.code;

	switch (fileEvent)
	{
	case vx::FileMessage::Scene_Loaded:
	{
		auto scene = (Scene*)msg.arg2.ptr;

		loadScene(scene);

		//TaskLoadScene data;
		//data.ptr = pScene;

		//queueUpdateTask(type, (u8*)&data, sizeof(TaskLoadScene));
	}break;
	case vx::FileMessage::EditorScene_Loaded:
	{
		/*auto pScene = (Scene*)evt.arg2.ptr;

		RenderUpdateTask task;
		task.type = RenderUpdateTask::Type::LoadScene;

		TaskLoadScene data;
		data.ptr = pScene;
		data.editor = true;

		queueUpdateTask(task, (u8*)&data, sizeof(TaskLoadScene));*/
	}break;
	default:
		break;
	}
}

void RenderLayerGame::loadScene(Scene* scene)
{
	auto lights = scene->getLights();
	auto lightCount = scene->getLightCount();
	auto device = m_device->getDevice();

	m_lightManager.loadSceneLights(lights, lightCount, device, m_resourceManager, m_uploadManager);
}

D3D12_DRAW_INDEXED_ARGUMENTS RenderLayerGame::addMeshInstance(const MeshInstance &meshInstance, u32* gpuIndex)
{
	auto material = meshInstance.getMaterial();
	u32 materialIndex = 0;
	u32 materialSlices = 0;
	m_materialManager->addMaterial(material, m_resourceAspect, m_uploadManager, &materialIndex, &materialSlices);

	D3D12_DRAW_INDEXED_ARGUMENTS cmd;
	m_meshManager.addMeshInstance(meshInstance, materialIndex, m_resourceAspect, m_resourceManager, m_uploadManager, &cmd);

	auto meshTransform = meshInstance.getTransform();
	updateTransform(meshTransform, cmd.StartInstanceLocation);

	auto materialBuffer = m_resourceManager->getBuffer(L"materialBuffer");
	auto materialOffset = sizeof(u32) * materialIndex;
	m_uploadManager->pushUploadBuffer((u8*)&materialSlices, materialBuffer->get(), materialOffset, sizeof(u32), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	m_drawCommandMesh.uploadDrawCommand(cmd.StartInstanceLocation, cmd, m_uploadManager);

	auto instanceCount = m_meshManager.getInstanceCount();
	m_drawCommandMesh.setCount(instanceCount, m_uploadManager);

	*gpuIndex = cmd.StartInstanceLocation;
	m_meshManager.updateResourceViews(m_resourceManager);

	return cmd;
}

void RenderLayerGame::addStaticMeshInstance(const MeshInstance &instance, const vx::StringID &sid)
{
	u32 gpuIndex = 0;
	auto cmd = addMeshInstance(instance, &gpuIndex);

	auto meshSid = instance.getMeshSid();
	auto transform = instance.getTransform();
	auto bounds = instance.getBounds();

	if (bounds.min.x == FLT_MAX)
	{
		auto mesh = m_resourceAspect->getMesh(meshSid);

		auto vertexCount = mesh->getMesh().getVertexCount();
		auto vertices = mesh->getMesh().getVertices();

		for (u32 i = 0; i < vertexCount; ++i)
		{
			bounds.min = vx::min(bounds.min, vertices[i].position);
			bounds.max = vx::max(bounds.max, vertices[i].position);
		}
	}

	m_lightManager.addStaticMeshInstance(cmd, bounds, m_uploadManager);
}

void RenderLayerGame::createActorGpuIndex(CreateActorData* data)
{
	auto actorTransform = data->getTransform();
	auto qRotation = vx::loadFloat4(actorTransform.m_qRotation);

	u32 materialIndex = 0;
	u32 materialSlices = 0;
	m_materialManager->addMaterial(data->getMaterialSid(), m_resourceAspect, m_uploadManager, &materialIndex, &materialSlices);

	D3D12_DRAW_INDEXED_ARGUMENTS cmd;
	m_meshManager.addMeshInstance(data->getActorSid(), data->getMeshSid(), materialIndex, m_resourceAspect, m_resourceManager, m_uploadManager, &cmd);
	data->setGpu(cmd.StartInstanceLocation);

	vx::Message e;
	e.arg1.ptr = data;
	e.code = (u32)IngameMessage::Gpu_AddedActor;
	e.type = vx::MessageType::Ingame;

	m_msgManager->addMessage(e);

	updateTransform(actorTransform, cmd.StartInstanceLocation);

	auto materialBuffer = m_resourceManager->getBuffer(L"materialBuffer");
	auto materialOffset = sizeof(u32) * materialIndex;
	m_uploadManager->pushUploadBuffer((u8*)&materialSlices, materialBuffer->get(), materialOffset, sizeof(u32), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	m_drawCommandMesh.uploadDrawCommand(cmd.StartInstanceLocation, cmd, m_uploadManager);

	auto instanceCount = m_meshManager.getInstanceCount();
	m_drawCommandMesh.setCount(instanceCount, m_uploadManager);

	m_meshManager.updateResourceViews(m_resourceManager);
}

void RenderLayerGame::updateTransform(const vx::Transform &meshTransform, u32 index)
{
	auto meshRotation = vx::loadFloat4(meshTransform.m_qRotation);

	vx::TransformGpu gpuTransform;
	gpuTransform.translation = meshTransform.m_translation;
	gpuTransform.scaling = 1.0f;
	gpuTransform.packedQRotation = GpuFunctions::packQRotation(meshRotation);
	updateTransformStatic(gpuTransform, index);

	auto transformBufferPrev = m_resourceManager->getBuffer(L"transformBufferPrev");
	auto transformOffset = sizeof(vx::TransformGpu) * index;
	m_uploadManager->pushUploadBuffer((u8*)&gpuTransform, transformBufferPrev->get(), transformOffset, sizeof(vx::TransformGpu), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void RenderLayerGame::updateTransformStatic(const vx::TransformGpu &transform, u32 index)
{
	auto transformBuffer = m_resourceManager->getBuffer(L"transformBuffer");

	auto transformOffset = sizeof(vx::TransformGpu) * index;
	m_uploadManager->pushUploadBuffer((u8*)&transform, transformBuffer->get(), transformOffset, sizeof(vx::TransformGpu), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void RenderLayerGame::updateTransformDynamic(const vx::TransformGpu &transform, u32 index)
{
	updateTransformStatic(transform, index);

	copyTransform(index);
}

void RenderLayerGame::taskUpdateDynamicTransforms(const u8* p, u32* offset)
{
	auto data = (RenderUpdateDataTransforms*)p;
	u32 count = data->count;

	auto transforms = data->transforms;

	u32* indices = data->indices;

	for (u32 i = 0; i < count; ++i)
	{
		auto index = indices[i];
		auto &transform = transforms[i];

		updateTransformDynamic(transform, index);
	}

	*offset += sizeof(RenderUpdateDataTransforms) + (sizeof(vx::TransformGpu) + sizeof(u32)) * count;
}


void RenderLayerGame::copyTransform(u32 index)
{
	auto transformBuffer = m_resourceManager->getBuffer(L"transformBuffer");
	auto transformBufferPrev = m_resourceManager->getBuffer(L"transformBufferPrev");

	u64 offset = sizeof(vx::TransformGpu) * index;

	m_copyManager->pushCopyBuffer(transformBuffer->get(), offset, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, sizeof(vx::TransformGpu), transformBufferPrev->get(), offset, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void RenderLayerGame::addDynamicMeshInstance(CreateDynamicMeshData* data)
{
	auto &instance = *(data->getMeshInstance());

	u32 gpuIndex = 0;
	addMeshInstance(instance, &gpuIndex);

	data->setGpuIndex(gpuIndex);

	vx::Message e;
	e.code = (u32)IngameMessage::Gpu_AddedDynamicMesh;
	e.type = vx::MessageType::Ingame;
	e.arg1.ptr = data;

	m_msgManager->addMessage(e);
}

void RenderLayerGame::queueUpdate(const RenderUpdateTaskType type, const u8* data, u32 dataSize)
{
	switch (type)
	{
	case RenderUpdateTaskType::UpdateCamera:
		break;
	case RenderUpdateTaskType::UpdateDynamicTransforms:
	{
		u32 offset = 0;
		taskUpdateDynamicTransforms(data, &offset);
	}break;
	case RenderUpdateTaskType::UpdateText:
		break;
	default:
		break;
	}
}