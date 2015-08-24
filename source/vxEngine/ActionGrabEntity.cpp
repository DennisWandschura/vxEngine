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

		auto humanVel = m_human->m_velocity;
		pose1.p.x += humanVel.x;
		pose1.p.z += humanVel.z;

		m_joint->setLocalPose(physx::PxJointActorIndex::eACTOR1, pose1);
		m_entity->m_rigidDynamic->wakeUp();
	}
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
	if (m_grabbed)
	{
		m_joint->release();
		m_entity->m_rigidDynamic->wakeUp();
		
		m_joint = nullptr;
		m_grabbed = false;
	}
}