/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include"FbxFactory.h"
#include <fbxsdk.h>
#include <memory>

#include <PxPhysicsAPI.h>
#include <vxLib/Graphics/Mesh.h>
#include <vxEngineLib/Animation.h>
#include <vector>
#include <map>
#include <vxLib/File/File.h>
#include <vxLib/File/FileHeader.h>
#include <vxEngineLib/MeshFile.h>
#include <vxEngineLib/FileFactory.h>
#include <vxLib/util/CityHash.h>
#include <vxEngineLib/AnimationFile.h>
#include <vxEngineLib/debugPrint.h>
#include <vxEngineLib/ArrayAllocator.h>
#include <vxEngineLib/managed_ptr.h>
#include <vxResourceAspect/ResourceManager.h>
#include <vxLib/ScopeGuard.h>

FbxFactory::FbxFactory()
	:m_pFbxManager(nullptr),
	m_pIOSettings(nullptr)
{
	m_pFbxManager = FBXSDK_NAMESPACE::FbxManager::Create();

	m_pIOSettings = FBXSDK_NAMESPACE::FbxIOSettings::Create(m_pFbxManager, IOSROOT);
	m_pFbxManager->SetIOSettings(m_pIOSettings);
}

FbxFactory::~FbxFactory()
{
	m_pIOSettings = nullptr;

	m_pFbxManager->Destroy();
	m_pFbxManager = nullptr;
}
/*void FbxFactory::loadMesh(const char *name, const FBXSDK_NAMESPACE::FbxMesh *pMesh, sorted_vector<Node, NodeCmp> *pNodes)
{
	//Node meshNode(, Node::ACTIVE);
	//meshNode.addData();
	auto polygonCount = pMesh->GetPolygonCount();
	//auto pControlPoints = pMesh->GetControlPoints();

	U32 *pVertexCount = new U32();
	*pVertexCount = polygonCount * 3;
	DirectX::XMFLOAT3 *pVertices = new DirectX::XMFLOAT3[(*pVertexCount)];
	DirectX::XMFLOAT3 *pNormals = new DirectX::XMFLOAT3[(*pVertexCount)];
	U32 *pIndices = new U32[(*pVertexCount)];

	if (!pVertexCount || !pVertices || !pNormals || !pIndices)
		return;

	U32 iIndex = 0, vIndex = 0;
	for (auto polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
	{
		auto polygonSize = pMesh->GetPolygonSize(polygonIndex);

		assert(polygonSize == 3);

		//auto polygonVertexIndex = pMesh->GetPolygonVertexIndex(polygonIndex);

		for (auto i = 0; i < polygonSize; ++i)
		{
			auto controlPointIndex = pMesh->GetPolygonVertex(polygonIndex, i);
			auto controlPoint = pMesh->GetControlPointAt(controlPointIndex);

			//auto vertexIndex = pMesh->GetPolygonVertices()[polygonVertexIndex + i];

			FbxVector2 uv;
			bool b;
			pMesh->GetPolygonVertexUV(polygonIndex, i, "map1", uv, b);

			FbxVector4 normal;
			pMesh->GetPolygonVertexNormal(polygonIndex, i, normal);

			pVertices[vIndex].x = controlPoint.mData[0];
			pVertices[vIndex].y = controlPoint.mData[1];
			pVertices[vIndex].z = controlPoint.mData[2];

			pNormals[vIndex].x = normal.mData[0];
			pNormals[vIndex].y = normal.mData[1];
			pNormals[vIndex].z = normal.mData[2];

			pIndices[iIndex] = vIndex;

			++vIndex;
			++iIndex;
		}
	}

	Node meshNode(name, Node::ACTIVE);
	meshNode.addData("vertexCount", pVertexCount);
	meshNode.addData("vertices", pVertices);
	meshNode.addData("normals", pNormals);
	meshNode.addData("indices", pIndices);

	auto it = pNodes->insert(std::move(meshNode));

	m_pEventQueue->addEvent(Event(EventType::ADDED_MESH, 0, &it.first, nullptr));
}
*/

void loadAnimLayer(FBXSDK_NAMESPACE::FbxAnimStack* animStack, int index)
{
	auto animLayer = animStack->GetMember<FBXSDK_NAMESPACE::FbxAnimLayer>(index);

	auto count = animLayer->GetReferencedByCount();
	printf("	%d\n", count);
}

void getKeys(const FBXSDK_NAMESPACE::FbxAnimCurve* animCurve, FBXSDK_NAMESPACE::FbxNode* meshNode, std::map<int, FBXSDK_NAMESPACE::FbxTime>* frameTimes)
{
	if (animCurve == nullptr)
		return;

	auto keyCount = animCurve->KeyGetCount();
	//printf("	keyCount: %d\n", keyCount);
	for (int i = 0; i < keyCount; ++i)
	{
		auto key = animCurve->KeyGet(i);
		auto time = key.GetTime();

		//printf("		value: %f, second: %d, frame: %d\n", key.GetValue(), time.GetSecondCount(), );
		frameTimes->insert(std::make_pair(time.GetFrameCount(), time));
	}
}

int getTranslationKeyFrames(FBXSDK_NAMESPACE::FbxNode* meshNode, FBXSDK_NAMESPACE::FbxAnimLayer* animLayer, std::vector<vx::float3>* tranlations)
{
	auto lAnimCurveX = meshNode->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_X);
	auto lAnimCurveY = meshNode->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Y);
	auto lAnimCurveZ = meshNode->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Z);

	auto keyCountX = lAnimCurveX->KeyGetCount();
	auto keyCountY = lAnimCurveY->KeyGetCount();
	auto keyCountZ = lAnimCurveZ->KeyGetCount();

	VX_ASSERT(keyCountX == keyCountY);
	VX_ASSERT(keyCountZ == keyCountY);

	auto keyCount = keyCountX;

	tranlations->reserve(keyCount);

	for (int i = 0; i < keyCount; ++i)
	{
		vx::float3 t;

		auto keyX = lAnimCurveX->KeyGet(i);
		auto keyY = lAnimCurveX->KeyGet(i);
		auto keyZ = lAnimCurveX->KeyGet(i);
	}

	return keyCount;
}

void getAnimationLayers(FBXSDK_NAMESPACE::FbxAnimStack* animStack, FBXSDK_NAMESPACE::FbxNode* meshNode, std::unique_ptr<vx::AnimationLayer[]>* animationLayers, u32* layerCount)
{
	auto animLayerCount = animStack->GetMemberCount<FBXSDK_NAMESPACE::FbxAnimLayer>();

	std::vector<vx::AnimationLayer> layers;
	layers.reserve(animLayerCount);
	for (int j = 0; j < animLayerCount; ++j)
	{
		FBXSDK_NAMESPACE::FbxAnimLayer* animLayer = animStack->GetMember<FBXSDK_NAMESPACE::FbxAnimLayer>(j);

		std::map<int, FBXSDK_NAMESPACE::FbxTime> frameTimes;

		//puts("translation:");
		auto lAnimCurveX = meshNode->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_X);
		auto lAnimCurveY = meshNode->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		auto lAnimCurveZ = meshNode->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Z);

		getKeys(lAnimCurveX, meshNode, &frameTimes);
		getKeys(lAnimCurveY, meshNode, &frameTimes);
		getKeys(lAnimCurveZ, meshNode, &frameTimes);
		//puts("rotation:");
		lAnimCurveX = meshNode->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_X);
		lAnimCurveY = meshNode->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		lAnimCurveZ = meshNode->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Z);

		getKeys(lAnimCurveX, meshNode, &frameTimes);
		getKeys(lAnimCurveY, meshNode, &frameTimes);
		getKeys(lAnimCurveZ, meshNode, &frameTimes);

		auto frameCount = frameTimes.size();

		auto animFrames = std::make_unique<vx::AnimationSample[]>(frameCount);
		u32 frameIndex = 0;
		f32 frameRate = 30.0f;
		for (auto &it : frameTimes)
		{
			frameRate = it.second.GetFrameRate(FBXSDK_NAMESPACE::FbxTime::EMode::eDefaultMode);

			auto &transform = meshNode->EvaluateGlobalTransform(it.second);
			auto translation = transform.GetT();
			auto rotation = transform.GetR();

			__m128 angles = { (f32)rotation[0], (f32)rotation[1], (f32)rotation[2], 0.0f };
			angles = vx::degToRad(angles);
			auto qRotation = vx::quaternionRotationRollPitchYawFromVector(angles);
			//printf("frame: %d, framerate: %d\n", it.first, it.second.GetFrameRate(FBXSDK_NAMESPACE::FbxTime::EMode::eDefaultMode));
			//printf("	translation: %f %f %f\n", translation[0], translation[1], translation[2]);
			//printf("	rotation: %f %f %f\n", rotation[0], rotation[1], rotation[2]);

			animFrames[frameIndex].transform.m_translation = vx::float3(translation[0], translation[1], translation[2]);
			vx::storeFloat4(&animFrames[frameIndex].transform.m_qRotation, qRotation);
			animFrames[frameIndex].transform.m_scaling = 1.0f;
			animFrames[frameIndex].frame = it.first;
			++frameIndex;
		}

		if (frameCount != 0)
		{
			vx::AnimationLayer animLayer;
			animLayer.frameCount = frameIndex;
			animLayer.frameRate = frameRate;
			animLayer.samples = std::move(animFrames);

			layers.push_back(std::move(animLayer));
		}
	}

	*layerCount = layers.size();
	*animationLayers = std::make_unique<vx::AnimationLayer[]>(layers.size());
	for (u32 i = 0; i < layers.size(); ++i)
	{
		(*animationLayers)[i] = std::move(layers[i]);
	}
}

bool createPhysXMesh(PhsyxMeshType meshType, const vx::Mesh &mesh, physx::PxDefaultMemoryOutputStream* writeBuffer, physx::PxCooking* cooking)
{
	auto vertexCount = mesh.getVertexCount();
	auto vertices = mesh.getVertices();

	if (meshType == PhsyxMeshType::Triangle)
	{
		auto indexCount = mesh.getIndexCount();
		auto indices = mesh.getIndices();

		physx::PxTriangleMeshDesc meshDesc;
		meshDesc.points.count = vertexCount;
		meshDesc.points.stride = sizeof(vx::MeshVertex);
		meshDesc.points.data = vertices;

		meshDesc.triangles.count = indexCount / 3;
		meshDesc.triangles.stride = 3 * sizeof(u32);
		meshDesc.triangles.data = indices;

		return cooking->cookTriangleMesh(meshDesc, *writeBuffer);
	}
	else
	{
		physx::PxConvexMeshDesc convexDesc;
		convexDesc.points.count = vertexCount;
		convexDesc.points.stride = sizeof(vx::MeshVertex);
		convexDesc.points.data = vertices;
		convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;
		convexDesc.vertexLimit = 256;

		return cooking->cookConvexMesh(convexDesc, *writeBuffer);
	}
}

bool FbxFactory::loadFile(const char *fbxFile, const std::string &saveDir, const std::string &animDir, PhsyxMeshType meshType, physx::PxCooking* cooking, 
	std::vector<vx::FileHandle>* meshFiles, std::vector<vx::FileHandle>* animFiles, ResourceManager<vx::MeshFile>* meshManager)
{
	FbxImporter* lImporter = FbxImporter::Create(m_pFbxManager, "");

	// Use the first argument as the filename for the importer.
	if (!lImporter->Initialize(fbxFile, -1, m_pFbxManager->GetIOSettings()))
	{
		printf("Call to FbxImporter::Initialize() failed.\n");
		printf("%s\n", fbxFile);
		printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
		return false;
	}

	SCOPE_EXIT
	{
		lImporter->Destroy();
		lImporter = nullptr;
	};

	FbxScene* lScene = FbxScene::Create(m_pFbxManager, fbxFile);
	// Import the contents of the file into the scene.
	lImporter->Import(lScene);

	// The file is imported, so get rid of the importer.
	//lImporter->Destroy();

	auto &globalSettings = lScene->GetGlobalSettings();
	//auto unit = globalSettings.GetOriginalSystemUnit();
	auto systemUnit = globalSettings.GetSystemUnit();

	//auto scale = 1.0 / systemUnit.GetScaleFactor();

	/*if ((systemUnit.GetScaleFactor() - 1.0) > 0.00001)
	{
		FBXSDK_NAMESPACE::FbxSystemUnit OurSystemUnit(1.0);
		OurSystemUnit.ConvertScene(lScene);
	}*/

	//auto newunit = globalSettings.GetSystemUnit();

	// extract meshes
	FBXSDK_NAMESPACE::FbxNode *lRootNode = lScene->GetRootNode();
	if (!lRootNode)
	{
		lScene->Destroy();
		return false;
	}

	//auto rootScale = lRootNode->LclScaling.Get();

	int animStackCount = lScene->GetSrcObjectCount<FBXSDK_NAMESPACE::FbxAnimStack>();
	auto animStack = lScene->GetSrcObject<FBXSDK_NAMESPACE::FbxAnimStack>(0);

	int meshCount = lScene->GetSrcObjectCount<FBXSDK_NAMESPACE::FbxMesh>();
	for (int i = 0; i < meshCount; ++i)
	{
		auto pMesh = lScene->GetSrcObject<FBXSDK_NAMESPACE::FbxMesh>(i);

		auto meshNode = pMesh->GetNode();
		auto transformMatrix = meshNode->EvaluateGlobalTransform();
		auto scaleFactor = transformMatrix.GetS();
		//printf("Mesh: %s\n", pMesh->GetNode()->GetName());
		//auto transform = meshNode->EvaluateGlobalTransform();

		auto elementTangentCount = pMesh->GetElementTangentCount();
		auto binormals = pMesh->GetElementBinormal();
		auto tangents = pMesh->GetElementTangent();
		auto tangentMappingMode = tangents->GetMappingMode();
		VX_ASSERT(tangentMappingMode == FBXSDK_NAMESPACE::FbxLayerElement::EMappingMode::eByPolygonVertex);
		auto tangentReferenceMode = tangents->GetReferenceMode();
		VX_ASSERT(tangentReferenceMode == FBXSDK_NAMESPACE::FbxLayerElement::eDirect);
		auto &tangentsArray = tangents->GetDirectArray();

		VX_ASSERT(binormals->GetMappingMode() == FBXSDK_NAMESPACE::FbxLayerElement::EMappingMode::eByPolygonVertex);
		VX_ASSERT(binormals->GetReferenceMode() == FBXSDK_NAMESPACE::FbxLayerElement::eDirect);
		auto &bitangentArray = binormals->GetDirectArray();

		auto polygonCount = pMesh->GetPolygonCount();
		auto vertexCount = polygonCount * 3;

		u32 vertexIndex = 0;
		u32 indexCount = 0;

		std::vector<vx::MeshVertex> meshVertices;
		meshVertices.reserve(vertexCount);

		std::vector<u32> meshIndices;
		meshIndices.reserve(vertexCount);

		//std::vector<u32> fbxIndices;
		//fbxIndices.reserve(vertexCount);

		auto controlPointCount = pMesh->GetControlPointsCount();

		auto fbxVertices = pMesh->GetControlPoints();

		for (auto polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
		{
			auto polygonSize = pMesh->GetPolygonSize(polygonIndex);
			VX_ASSERT(polygonSize == 3);

			for (auto i = 0; i < polygonSize; ++i)
			{
				auto controlPointIndex = pMesh->GetPolygonVertex(polygonIndex, i);
				auto position = fbxVertices[controlPointIndex];

				//fbxIndices.push_back(controlPointIndex);

				auto tangent = tangentsArray.GetAt(vertexIndex);
				auto bitangent = bitangentArray.GetAt(vertexIndex);

				FbxVector2 uv;
				bool unmapped;
				bool b = pMesh->GetPolygonVertexUV(polygonIndex, i, "map1", uv, unmapped);

				FbxVector4 normal;
				pMesh->GetPolygonVertexNormal(polygonIndex, i, normal);

				vx::MeshVertex vertex;
				vertex.position = vx::float3(position[0] * scaleFactor[0], position[1] * scaleFactor[1], position[2] * scaleFactor[2]);
				vertex.normal = vx::float3(normal[0], normal[1], normal[2]);
				vertex.tangent = vx::float3(tangent[0], tangent[1], tangent[2]);
				vertex.bitangent = vx::float3(bitangent[0], bitangent[1], bitangent[2]);
				vertex.texCoords = vx::float2(uv[0], uv[1]);

				meshVertices.push_back(vertex);
				meshIndices.push_back(vertexIndex);

				++vertexIndex;
				++indexCount;
			}
		}

		std::vector<vx::float3> points;
		points.reserve(controlPointCount);
		for (u32 i = 0; i < controlPointCount; ++i)
		{
			auto &it = fbxVertices[i];
			points.push_back(vx::float3(it[0], it[1], it[2]));
		}

		std::vector<vx::MeshVertex> newVertices;
		newVertices.reserve(vertexCount);

		std::vector<std::pair<u32, u32>> duplicateIndices;
		duplicateIndices.reserve(vertexCount);

		for (u32 i = 0; i < vertexCount; ++i)
		{
			u32 foundIndex = 0;
			auto &currentVertex = meshVertices[i];
			bool found = false;
			for (auto &it : newVertices)
			{
				if (memcmp(&it, &currentVertex, sizeof(vx::MeshVertex)) == 0)
				{
					found = true;
					break;
				}

				++foundIndex;
			}

			if (!found)
			{
				auto newIndex = newVertices.size();
				auto oldIndex = i;
				for (auto &current : meshIndices)
				{
					if (current == oldIndex)
					{
						current = newIndex;
					}
				}

				newVertices.push_back(currentVertex);
			}
			else
			{
				auto newIndex = foundIndex;
				auto oldIndex = i;
				for (auto &current : meshIndices)
				{
					if (current == oldIndex)
					{
						current = newIndex;
					}
				}
			}
		}

		vertexCount = newVertices.size();
		meshVertices.swap(newVertices);

		u32 verticesSizeInBytes = sizeof(vx::MeshVertex) * vertexCount;
		u32 indexSizeInBytes = sizeof(u32) * indexCount;
		std::unique_lock<std::mutex> dataLock;
		auto dataAlloc = meshManager->lockDataAllocator(&dataLock);
		managed_ptr<u8[]> meshDataPtr = dataAlloc->allocate<u8[]>(verticesSizeInBytes + indexSizeInBytes, 4);
		dataLock.unlock();

		memcpy(meshDataPtr.get(), meshVertices.data(), verticesSizeInBytes);
		memcpy(meshDataPtr.get() + verticesSizeInBytes, meshIndices.data(), indexSizeInBytes);

		vx::Mesh mesh(meshDataPtr.get(), vertexCount, indexCount);

		physx::PxDefaultMemoryOutputStream writeBuffer;
		if (!createPhysXMesh(meshType, mesh, &writeBuffer, cooking))
		{
			printf("Error creating physx mesh\n");
			return false;
		}

		auto physxSize = writeBuffer.getSize();
		dataLock.lock();
		managed_ptr<u8[]> physxData = dataAlloc->allocate<u8[]>(physxSize, 4);
		memcpy(physxData.get(), writeBuffer.getData(), physxSize);
		dataLock.unlock();

		dataLock.lock();
		vx::MeshFile meshFile(vx::MeshFile::getGlobalVersion(), std::move(mesh), std::move(meshDataPtr), std::move(physxData), physxSize, meshType);
		dataLock.unlock();

		std::string fileName = pMesh->GetNode()->GetName();
		std::string meshFileName = fileName + ".mesh";
		auto meshFileNameWithPath = saveDir + meshFileName;

		vx::FileFactory::saveToFile(meshFileNameWithPath.c_str(), &meshFile);
		meshFiles->push_back(vx::FileHandle(meshFileName.c_str()));
		vx::verboseChannelPrintF(0, vx::debugPrint::Channel_FileAspect, "Converted fbx to mesh %s\n", meshFileNameWithPath.c_str());

		std::unique_ptr<vx::AnimationLayer[]> animationLayers;
		u32 animationLayerCount = 0;
		getAnimationLayers(animStack, meshNode, &animationLayers, &animationLayerCount);

		if (animationLayerCount != 0)
		{
			std::string animFileName = fileName + ".animation";
			std::string animFileWithPath = animDir + animFileName;

			vx::Animation animation;
			animation.layerCount = animationLayerCount;
			animation.layers = std::move(animationLayers);

			vx::AnimationFile animationFile(vx::AnimationFile::getGlobalVersion(), std::move(animation), animFileName.c_str());

			vx::FileFactory::saveToFile(animFileWithPath.c_str(), &animationFile);
			vx::verboseChannelPrintF(0, vx::debugPrint::Channel_FileAspect, "Saved animation %s\n", animFileName.c_str());

			animFiles->push_back(vx::FileHandle(animFileName.c_str()));
		}

		//printf("animation layers: %u\n", animationLayerCount);
		//printf("vertexCount: %u\n", vertexCount);
	}

	return true;
}