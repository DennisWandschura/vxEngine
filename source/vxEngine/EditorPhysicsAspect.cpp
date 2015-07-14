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
#include "EditorPhysicsAspect.h"
#include <PxPhysicsAPI.h>
#include <vxEngineLib/MeshInstance.h>
#include <vxResourceAspect/FileAspect.h>
#include <vxEngineLib/Locator.h>
#include <vxEngineLib/Material.h>
#include <vxEngineLib/EventTypes.h>
#include <vxEngineLib/Event.h>
#include <vxEngineLib/FileEvents.h>
#include <vxEngineLib/EditorScene.h>
#include <vxEngineLib/EditorMeshInstance.h>
#include <vxEngineLib/MeshFile.h>

namespace Editor
{
	PhysicsAspect::PhysicsAspect()
		: ::PhysicsAspect()
	{

	}

	PhysicsAspect::~PhysicsAspect()
	{

	}

	void PhysicsAspect::handleFileEvent(const vx::Event &evt)
	{
		auto fileEvent = (vx::FileEvent)evt.code;
		switch (fileEvent)
		{
		case vx::FileEvent::EditorScene_Loaded:
			processScene((Editor::Scene*)evt.arg2.ptr);
			break;
		default:
			break;
		}
	}

	void PhysicsAspect::handleEvent(const vx::Event &evt)
	{
		switch (evt.type)
		{
		case(vx::EventType::File_Event) :
			handleFileEvent(evt);
			break;
		default:
			break;
		}
	}

	void PhysicsAspect::processScene(const Editor::Scene* pScene)
	{
		m_pScene->lockWrite();

		auto &meshes = pScene->getMeshes();
		auto keys = meshes.keys();

		for (auto i = 0u; i < meshes.size(); ++i)
		{
			auto meshFileRef = meshes[i];
			if (!addMesh(keys[i], (*meshFileRef)))
			{
				printf("Error processing mesh %llu\n", keys[i].value);
				VX_ASSERT(false);
			}
		}

		auto numInstances = pScene->getMeshInstanceCount();
		auto pMeshInstances = pScene->getMeshInstancesEditor();

		auto sceneMaterials = pScene->getMaterials();
		auto materialCount = pScene->getMaterialCount();
		for (auto i = 0u; i < materialCount; ++i)
		{
			auto &currentMaterial = sceneMaterials[i];

			auto sid = (*currentMaterial).getSid();
			auto pMaterial = m_pPhysics->createMaterial((*currentMaterial).getStaticFriction(), (*currentMaterial).getDynamicFriction(), (*currentMaterial).getRestitution());
			assert(pMaterial);
			m_physxMaterials.insert(sid, pMaterial);
		}

		for (auto i = 0u; i < numInstances; ++i)
		{
			auto &instance = pMeshInstances[i];
			auto sid = instance.getAnimationSid();
			addMeshInstance(instance.getMeshInstance());
		}

		m_pScene->unlockWrite();
	}

	bool PhysicsAspect::editorGetStaticMeshInstancePosition(const vx::StringID &sid, vx::float3* p) const
	{
		bool result = false;

		auto it = m_staticMeshInstances.find(sid);
		if (it != m_staticMeshInstances.end())
		{
			physx::PxTransform transform = (*it)->getGlobalPose();

			p->x = transform.p.x;
			p->y = transform.p.y;
			p->z = transform.p.z;

			result = true;
		}

		return result;
	}

	void PhysicsAspect::editorSetStaticMeshInstanceTransform(const ::MeshInstance &meshInstance, const vx::StringID &sid)
	{
		auto it = m_staticMeshInstances.find(sid);
		if (it != m_staticMeshInstances.end())
		{
			auto instanceTransform = meshInstance.getTransform();

			auto qRotation = vx::loadFloat4(instanceTransform.m_qRotation);

			physx::PxTransform transform;
			transform.p.x = instanceTransform.m_translation.x;
			transform.p.y = instanceTransform.m_translation.y;
			transform.p.z = instanceTransform.m_translation.z;
			_mm_storeu_ps(&transform.q.x, qRotation);

			(*it)->setGlobalPose(transform);
		}
	}

	void PhysicsAspect::editorAddMeshInstance(const ::MeshInstance &instance)
	{
		VX_ASSERT(false);
		//addMeshInstance(instance, PHysic);
	}

	void PhysicsAspect::editorSetStaticMeshInstanceMesh(const ::MeshInstance &meshInstance)
	{
		auto sid = meshInstance.getNameSid();
		auto rigidStaticIt = m_staticMeshInstances.find(sid);

		auto meshSid = meshInstance.getMeshSid();
		auto fileAspect = Locator::getFileAspect();
		auto meshFile = fileAspect->getMesh(meshSid);
		auto physxType = meshFile->getPhysxMeshType();

		auto &material = meshInstance.getMaterial();
		auto itPhysxMaterial = m_physxMaterials.find((*material).getSid());

		physx::PxShape* newShape = nullptr;
		switch (physxType)
		{
		case PhsyxMeshType::Triangle:
		{
			auto triangleMesh = getTriangleMesh(meshSid, (*meshFile));
			newShape = m_pPhysics->createShape(physx::PxTriangleMeshGeometry(triangleMesh), *(*itPhysxMaterial));
		}break;
		case PhsyxMeshType::Convex:
		{
			auto convexMesh = getConvexMesh(meshSid, (*meshFile));
			newShape = m_pPhysics->createShape(physx::PxConvexMeshGeometry(convexMesh), *(*itPhysxMaterial));
		}break;
		default:
			break;
		}

		auto shapeCount = (*rigidStaticIt)->getNbShapes();
		VX_ASSERT(shapeCount == 1);

		physx::PxShape* oldShape = nullptr;;
		(*rigidStaticIt)->getShapes(&oldShape, 1);

		puts("attach shape begin");
		(*rigidStaticIt)->attachShape(*newShape);
		(*rigidStaticIt)->detachShape(*oldShape);
		puts("end shape begin");
	}

	void PhysicsAspect::setPosition(const vx::float3 &position, physx::PxController* pController)
	{
		pController->setPosition(physx::PxExtendedVec3(position.x, position.y, position.z));
	}

	bool PhysicsAspect::createTriangleMesh(const vx::float3* positions, u32 vertexCount, const u32* indices, u32 indexCount, physx::PxDefaultMemoryOutputStream* writeBuffer)
	{
		physx::PxTriangleMeshDesc meshDesc;
		meshDesc.points.count = vertexCount;
		meshDesc.points.stride = sizeof(vx::float3);
		meshDesc.points.data = positions;

		meshDesc.triangles.count = indexCount / 3;
		meshDesc.triangles.stride = 3 * sizeof(u32);
		meshDesc.triangles.data = indices;

		return m_pCooking->cookTriangleMesh(meshDesc, *writeBuffer);
	}

	bool PhysicsAspect::createConvexMesh(const vx::float3* positions, u32 vertexCount, physx::PxDefaultMemoryOutputStream* writeBuffer)
	{
		physx::PxConvexMeshDesc convexDesc;
		convexDesc.points.count = vertexCount;
		convexDesc.points.stride = sizeof(vx::float3);
		convexDesc.points.data = positions;
		convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;
		convexDesc.vertexLimit = 256;

		return m_pCooking->cookConvexMesh(convexDesc, *writeBuffer);
	}

	bool PhysicsAspect::setMeshPhysxType(Reference<vx::MeshFile> &meshFile, PhsyxMeshType type, ArrayAllocator* meshDataAllocator)
	{
		auto &mesh = meshFile->getMesh();

		auto vertexCount = mesh.getVertexCount();
		auto vertices = mesh.getVertices();

		auto positions = vx::make_unique<vx::float3[]>(vertexCount);
		for (u32 i = 0; i < vertexCount; ++i)
		{
			positions[i] = vertices[i].position;
		}

		auto indexCount = mesh.getIndexCount();
		auto indices = mesh.getIndices();

		physx::PxDefaultMemoryOutputStream writeBuffer;

		bool result = false;
		switch (type)
		{
		case PhsyxMeshType::Triangle:
		{
			result = createTriangleMesh(positions.get(), vertexCount, indices, indexCount, &writeBuffer);
		}break;
		case PhsyxMeshType::Convex:
		{
			result = createConvexMesh(positions.get(), vertexCount, &writeBuffer);
		}break;
		default:
			break;
		}

		if (result)
		{
			auto dataSize = writeBuffer.getSize();
			auto data = writeBuffer.getData();

			managed_ptr<u8[]> physData = meshDataAllocator->allocate(dataSize, 4);
			memcpy(physData.get(), data, dataSize);

			meshFile->setPhysxMesh(std::move(physData), dataSize, type);
		}

		return result;
	}
}