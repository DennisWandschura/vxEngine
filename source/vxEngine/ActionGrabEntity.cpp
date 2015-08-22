#include "ActionGrabEntity.h"
#include "Entity.h"
#include <PxRigidDynamic.h>
#include <vxEngineLib/Locator.h>
#include "PhysicsAspect.h"
#include <extensions/PxSphericalJoint.h>

ActionGrabEntity::ActionGrabEntity(EntityHuman* human, EntityDynamic* entity)
	:m_human(human),
	m_entity(entity),
	m_joint(nullptr),
	m_grabbed(false)
{

}

ActionGrabEntity::~ActionGrabEntity()
{

}


void ActionGrabEntity::run()
{
	if (m_joint)
	{
		auto joint = m_joint;

		auto pose1 = m_joint->getLocalPose(physx::PxJointActorIndex::eACTOR1);
		//auto pose1 = m_joint->getLocalPose(physx::PxJointActorIndex::eACTOR1);

		auto humanVel = m_human->m_velocity;
		//pose0.p.y += 0.1f;

		//printf("%f\n", pose1.p.y);
		pose1.p.x += humanVel.x;
		pose1.p.z += humanVel.z;

		//printf("%f %f\n", pose1.p.x, pose1.p.z);

		m_joint->setLocalPose(physx::PxJointActorIndex::eACTOR1, pose1);
		m_entity->m_rigidDynamic->wakeUp();
		//m_joint->setLocalPose(physx::PxJointActorIndex::eACTOR1, pose1);
	}
	/*if (m_grabbed)
	{
		//auto humanPos = m_human->m_position;
		auto humanVel = m_human->m_velocity;

		physx::PxVec3 force;
		force.x = humanVel.x;
		force.y = 0.0f;
		force.z = humanVel.z;

		printf("%f %f\n", force.x, force.z);

		
		//m_entity->m_rigidDynamic->addForce(force, physx::PxForceMode::eVELOCITY_CHANGE);
		m_entity->m_rigidDynamic->getli
		//m_entity->m_rigidDynamic->clearForce(physx::PxForceMode::eVELOCITY_CHANGE);
		//

		//auto entityPos = m_entity->m_position;

		//auto dirToEntity = vx::normalize3(entityPos - humanPos);
		
		//printf();
	}*/
}

void ActionGrabEntity::startUse()
{
	if (!m_grabbed)
	{
		m_joint = Locator::getPhysicsAspect()->createSphericalJoint(m_entity->m_rigidDynamic);
		m_entity->m_rigidDynamic->wakeUp();
		m_grabbed = true;
	}
}

void ActionGrabEntity::stopUse()
{
	//m_entity->m_rigidDynamic->clearForce(physx::PxForceMode::eVELOCITY_CHANGE);
	if (m_grabbed)
	{
		
		m_joint->release();
		m_entity->m_rigidDynamic->wakeUp();
		
		m_joint = nullptr;
		m_grabbed = false;
	}
}