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
#include <vxResourceAspect/ResourceAspect.h>
#include <vxEngineLib/Locator.h>
#include <vxEngineLib/Material.h>
#include <vxEngineLib/MessageTypes.h>
#include <vxEngineLib/Message.h>
#include <vxEngineLib/FileMessage.h>
#include <vxEngineLib/EditorScene.h>
#include <vxEngineLib/EditorMeshInstance.h>
#include <vxEngineLib/MeshFile.h>
#include <vxEngineLib/Joint.h>

namespace Editor
{
	namespace PhysicsAspectCpp
	{
		void copy(const vx::Transform &transform, physx::PxTransform* physxTransform)
		{
			physxTransform->p.x = transform.m_translation.x;
			physxTransform->p.y = transform.m_translation.y;
			physxTransform->p.z = transform.m_translation.z;

			physxTransform->q.x = transform.m_qRotation.x;
			physxTransform->q.y = transform.m_qRotation.y;
			physxTransform->q.z = transform.m_qRotation.z;
			physxTransform->q.w = transform.m_qRotation.w;
		}

		void copy(const vx::float3 &src, physx::PxVec3* dst)
		{
			dst->x = src.x;
			dst->y = src.y;
			dst->z = src.z;
		}

		void copy(const vx::float4 &src, physx::PxQuat* dst)
		{
			dst->x = src.x;
			dst->y = src.y;
			dst->z = src.z;
			dst->w = src.w;
		}
	}

	PhysicsAspect::PhysicsAspect()
		: ::PhysicsAspect()
	{

	}

	PhysicsAspect::~PhysicsAspect()
	{

	}

	void PhysicsAspect::handleFileEvent(const vx::Message &evt)
	{
		auto fileEvent = (vx::FileMessage)evt.code;
		switch (fileEvent)
		{
		case vx::FileMessage::EditorScene_Loaded:
			processScene((Editor::Scene*)evt.arg2.ptr);
			break;
		default:
			break;
		}
	}

	void PhysicsAspect::handleMessage(const vx::Message &evt)
	{
		switch (evt.type)
		{
		case(vx::MessageType::File_Event) :
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
			void* outData = nullptr;
			auto &instance = pMeshInstances[i];
			::PhysicsAspect::addMeshInstanceImpl(instance.getMeshInstance(), &outData);
		}

		auto joints = pScene->getJoints();
		auto jointCount = pScene->getJointCount();
		for (auto i = 0u; i < jointCount; ++i)
		{
			createJoint(joints[i]);
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

	void PhysicsAspect::editorSetStaticMeshInstanceMesh(const ::MeshInstance &meshInstance)
	{
		auto sid = meshInstance.getNameSid();
		auto rigidStaticIt = m_staticMeshInstances.find(sid);

		auto meshSid = meshInstance.getMeshSid();
		auto resourceAspect = Locator::getResourceAspect();
		auto meshFile = resourceAspect->getMesh(meshSid);
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

			managed_ptr<u8[]> physData = meshDataAllocator->allocate<u8[]>(dataSize, 4);
			memcpy(physData.get(), data, dataSize);

			meshFile->setPhysxMesh(std::move(physData), dataSize, type);
		}

		return result;
	}

	bool PhysicsAspect::setMeshInstanceRigidBodyType(const vx::StringID &instanceSid, const ::MeshInstance &meshInstance, PhysxRigidBodyType rigidBodyType)
	{
		bool result = false;

		auto it = m_meshInstances.find(instanceSid);
		if (it != m_meshInstances.end())
		{
			auto oldType = *it;
			if (oldType != rigidBodyType)
			{
				auto transform = meshInstance.getTransform();

				physx::PxTransform pxTransform;
				pxTransform.p.x = transform.m_translation.x;
				pxTransform.p.y = transform.m_translation.y;
				pxTransform.p.z = transform.m_translation.z;

				pxTransform.q.x = transform.m_qRotation.x;
				pxTransform.q.y = transform.m_qRotation.y;
				pxTransform.q.z = transform.m_qRotation.z;
				pxTransform.q.w = transform.m_qRotation.w;

				switch (oldType)
				{
				case PhysxRigidBodyType::Static:
				{
					auto itBody = m_staticMeshInstances.find(instanceSid);
					auto shapeCount = (*itBody)->getNbShapes();
					auto shapes = vx::make_unique<physx::PxShape*[]>(shapeCount);
					(*itBody)->getShapes(shapes.get(), shapeCount);


					auto newBody = m_pPhysics->createRigidDynamic(pxTransform);
					for (auto i = 0u; i < shapeCount; ++i)
					{
						newBody->attachShape(*shapes[i]);
					}
					m_dynamicMeshInstances.insert(instanceSid, newBody);

					m_pScene->removeActor(*(*itBody));
					(*itBody)->release();
					m_staticMeshInstances.erase(itBody);

					m_pScene->addActor(*newBody);
				}break;
				case PhysxRigidBodyType::Dynamic:
				{
					auto itBody = m_dynamicMeshInstances.find(instanceSid);
					auto shapeCount = (*itBody)->getNbShapes();
					auto shapes = vx::make_unique<physx::PxShape*[]>(shapeCount);
					(*itBody)->getShapes(shapes.get(), shapeCount);


					auto newBody = m_pPhysics->createRigidStatic(pxTransform);
					for (auto i = 0u; i < shapeCount; ++i)
					{
						newBody->attachShape(*shapes[i]);
					}
					m_staticMeshInstances.insert(instanceSid, newBody);

					m_pScene->removeActor(*(*itBody));
					(*itBody)->release();
					m_dynamicMeshInstances.erase(itBody);

					m_pScene->addActor(*newBody);
				}break;
				default:
					VX_ASSERT(false);
					break;
				}

				*it = rigidBodyType;
				result = true;
			}
		}

		return result;
	}

	void PhysicsAspect::addMeshInstance(const ::MeshInstance &instance)
	{
		void* outData = nullptr;
		::PhysicsAspect::addMeshInstanceImpl(instance, &outData);
	}

	bool PhysicsAspect::createJoint(const Joint &joint)
	{
		auto ptr = ::PhysicsAspect::createJoint(joint);

		return (ptr != nullptr);
	}
}