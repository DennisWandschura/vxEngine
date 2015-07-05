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
			bool isTriangleMesh = false;
			auto pMeshFile = meshes[i];
			if (!processMesh(keys[i], pMeshFile, &isTriangleMesh))
			{
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
			addMeshInstance(pMeshInstances[i].getMeshInstance());
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
		addMeshInstance(instance);
	}

	void PhysicsAspect::editorSetStaticMeshInstanceMesh(const ::MeshInstance &meshInstance)
	{
		auto sid = meshInstance.getNameSid();
		auto rigidStaticIt = m_staticMeshInstances.find(sid);

		auto meshSid = meshInstance.getMeshSid();
		auto newTriangleMeshIt = m_physxMeshes.find(meshSid);
		auto newConvexMeshIt = m_physxConvexMeshes.find(meshSid);

		bool foundMesh = newTriangleMeshIt != m_physxMeshes.end() &&
			newConvexMeshIt != m_physxConvexMeshes.end();

		bool isTriangleMesh = false;
		if (!foundMesh)
		{
			puts("process mesh begin");
			auto fileAspect = Locator::getFileAspect();
			auto pMeshFile = fileAspect->getMesh(meshSid);

			if (!processMesh(meshSid, pMeshFile, &isTriangleMesh))
			{
				printf("error processing mesh\n");
				VX_ASSERT(false);
			}

			newTriangleMeshIt = m_physxMeshes.find(meshSid);
			newConvexMeshIt = m_physxConvexMeshes.find(meshSid);

			puts("process mesh end");
		}

		auto &material = meshInstance.getMaterial();
		auto itPhysxMaterial = m_physxMaterials.find((*material).getSid());

		puts("create shape begin");
		physx::PxShape* newShape = nullptr;
		if (isTriangleMesh)
		{
			newShape = m_pPhysics->createShape(physx::PxTriangleMeshGeometry(*newTriangleMeshIt), *(*itPhysxMaterial));
		}
		else
		{
			VX_ASSERT(newConvexMeshIt != m_physxConvexMeshes.end());
			newShape = m_pPhysics->createShape(physx::PxConvexMeshGeometry(*newConvexMeshIt), *(*itPhysxMaterial));
		}
		puts("create shape end");

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
}