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
#include "GBufferRenderer.h"
#include "RenderPassShadow.h"
#include "RenderPassZBuffer.h"
#include "RenderPassZBufferCreateMipmaps.h"
#include "RenderPassAo.h"
#include "RenderPassShading.h"
#include "RenderPassFinal.h"
#include <vxEngineLib/CreateDynamicMeshData.h>
#include "Device.h"
#include "CopyManager.h"

const u32 g_swapChainBufferCount{ 2 };
const u32 g_maxVertexCount{ 20000 };
const u32 g_maxIndexCount{ 40000 };
const u32 g_maxMeshInstances{ 128 };
const u32 g_maxLightCount{ 64 };

RenderLayerGame::RenderLayerGame(const RenderLayerGameDesc &desc)
	:m_renderPasses(),
	m_copyManager(desc.m_copyManager),
	m_commandAllocator(),
	m_lightManager(),
	m_camera(desc.m_camera),
	m_frustum(desc.m_frustum),
	m_drawCommandMesh(),
	m_device(desc.m_device),
	m_uploadManager(desc.m_uploadManager),
	m_msgManager(desc.m_msgManager),
	m_resourceManager(desc.m_resourceManager),
	m_materialManager(desc.m_materialManager),
	m_resourceAspect(desc.m_resourceAspect),
	m_meshManager()
{
}

RenderLayerGame::RenderLayerGame(RenderLayerGame &&rhs)
	:m_renderPasses(std::move(rhs.m_renderPasses)),
	m_copyManager(rhs.m_copyManager),
	m_commandAllocator(std::move(rhs.m_commandAllocator)),
	m_lightManager(std::move(rhs.m_lightManager)),
	m_camera(rhs.m_camera),
	m_frustum(rhs.m_frustum),
	m_drawCommandMesh(std::move(m_drawCommandMesh)),
	m_device(rhs.m_device),
	m_uploadManager(rhs.m_uploadManager),
	m_msgManager(rhs.m_msgManager),
	m_resourceManager(rhs.m_resourceManager),
	m_materialManager(rhs.m_materialManager),
	m_resourceAspect(rhs.m_resourceAspect),
	m_meshManager(std::move(rhs.m_meshManager))
{

}

RenderLayerGame::~RenderLayerGame()
{

}

void RenderLayerGame::createRenderPasses()
{
	pushRenderPass(std::make_unique<GBufferRenderer>(&m_commandAllocator, &m_drawCommandMesh));
	pushRenderPass(std::make_unique<RenderPassShadow>(&m_commandAllocator, &m_drawCommandMesh));
	pushRenderPass(std::make_unique<RenderPassZBuffer>(&m_commandAllocator));
	pushRenderPass(std::make_unique<RenderPassZBufferCreateMipmaps>(&m_commandAllocator));
	pushRenderPass(std::make_unique<RenderPassAO>(&m_commandAllocator));
	pushRenderPass(std::make_unique<RenderPassShading>(&m_commandAllocator));
	pushRenderPass(std::make_unique<RenderPassFinal>(&m_commandAllocator, m_device));
}

void RenderLayerGame::getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs)
{
	m_lightManager.getRequiredMemory(heapSizeBuffer, g_maxLightCount);

	m_drawCommandMesh.getRequiredMemory(g_maxMeshInstances, heapSizeBuffer);

	auto device = m_device->getDevice();
	for (auto &it : m_renderPasses)
	{
		it->getRequiredMemory(heapSizeBuffer, heapSizeTexture, heapSizeRtDs, device);
	}
}

bool RenderLayerGame::initialize(vx::StackAllocator* allocator)
{
	auto device = m_device->getDevice();
	if (!m_commandAllocator.create(D3D12_COMMAND_LIST_TYPE_DIRECT, device))
	{
		return false;
	}

	if (!m_lightManager.initialize(allocator, g_maxLightCount, m_resourceManager))
		return false;

	if (!m_meshManager.initialize(g_maxVertexCount, g_maxIndexCount, g_maxMeshInstances, m_resourceManager, device, allocator))
		return false;


	if (!m_drawCommandMesh.create(L"drawCmdBuffer", g_maxMeshInstances, m_resourceManager, m_device->getDevice()))
		return false;

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

	return true;
}

void RenderLayerGame::shudown()
{
	for (auto &it : m_renderPasses)
	{
		it->shutdown();
	}
	m_renderPasses.clear();

	m_meshManager.shutdown();

	m_copyManager = nullptr;
}

void RenderLayerGame::update()
{
	auto camPos = m_camera->getPosition();
	auto camRot = m_camera->getRotation();
	__m128 cameraPosition = _mm256_cvtpd_ps(camPos);
	__m128 cameraRotation = _mm256_cvtpd_ps(camRot);

	__m128 cameraDirection = { 0, 0, -1, 0 };
	cameraDirection = vx::quaternionRotation(cameraDirection, cameraRotation);

	m_lightManager.update(cameraPosition, cameraDirection, *m_frustum);
}

void RenderLayerGame::submitCommandLists(Graphics::CommandQueue* queue)
{
	auto hr = m_commandAllocator->Reset();
	VX_ASSERT(hr == 0);

	for (auto &it : m_renderPasses)
	{
		it->submitCommands(queue);
	}
}

u32 RenderLayerGame::getCommandListCount() const
{
	return m_renderPasses.size();
}

void RenderLayerGame::pushRenderPass(std::unique_ptr<RenderPass> &&renderPass)
{
	m_renderPasses.push_back(std::move(renderPass));
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
	m_uploadManager->pushUploadBuffer((u8*)&materialSlices, materialBuffer, materialOffset, sizeof(u32), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

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
	m_uploadManager->pushUploadBuffer((u8*)&materialSlices, materialBuffer, materialOffset, sizeof(u32), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	//auto cmdOffset = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS) * cmd.StartInstanceLocation;
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
	m_uploadManager->pushUploadBuffer((u8*)&gpuTransform, transformBufferPrev, transformOffset, sizeof(vx::TransformGpu), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void RenderLayerGame::updateTransformStatic(const vx::TransformGpu &transform, u32 index)
{
	auto transformBuffer = m_resourceManager->getBuffer(L"transformBuffer");

	auto transformOffset = sizeof(vx::TransformGpu) * index;
	m_uploadManager->pushUploadBuffer((u8*)&transform, transformBuffer, transformOffset, sizeof(vx::TransformGpu), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
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

	m_copyManager->pushCopyBuffer(transformBuffer, offset, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, sizeof(vx::TransformGpu), transformBufferPrev, offset, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
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