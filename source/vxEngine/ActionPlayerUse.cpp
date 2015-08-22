#include "ActionPlayerUse.h"
#include "Entity.h"
#include "ComponentActionManager.h"
#include "ComponentAction.h"
#include "ActionUseEntity.h"

ActionPlayerUse::ActionPlayerUse(EntityHuman* human, ComponentActionManager* components)
	:m_human(human),
	m_components(components),
	m_component(nullptr),
	m_keydown(false),
	m_keyup(false)
{

}

ActionPlayerUse::~ActionPlayerUse()
{

}

void ActionPlayerUse::run()
{
	const __m128 forward = { 0, 0, -1, 0 };

	if (m_keydown)
	{
		if (m_component == nullptr)
		{
			auto position = m_human->m_position;

			auto qRotation = vx::loadFloat4(m_human->m_qRotation);
			auto viewDir = vx::quaternionRotation(forward, qRotation);

			vx::float3 dir;
			vx::storeFloat3(&dir, viewDir);
			auto component = m_components->getComponent(position, dir);
			if (component)
			{
				m_component = component;

				//puts("player start use");
				m_component->action->startUse();
			}
		}

		m_keydown = false;
	}
	else if (m_keyup)
	{
		if (m_component)
		{
			m_component->action->stopUse();
			//puts("player stop use");
			m_component = nullptr;
		}

		m_keyup = false;
	}

	if (m_component)
	{
		
	}
}

bool ActionPlayerUse::isComplete() const
{
	return true;
}

void ActionPlayerUse::setKeyDown()
{
	m_keydown = true;
}

void ActionPlayerUse::setKeyUp()
{
	m_keyup = true;
}