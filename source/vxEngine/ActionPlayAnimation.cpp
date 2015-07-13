#include "ActionPlayAnimation.h"
#include <PxRigidDynamic.h>

ActionPlayAnimation::ActionPlayAnimation()
	:m_animation(),
	m_currentFrame(0),
	m_frameCount(0)
{

}

ActionPlayAnimation::~ActionPlayAnimation()
{

}

void ActionPlayAnimation::run()
{
	auto currentFrame = m_currentFrame;
	auto &layer = m_animation->layers[0];

	auto &currentSample = layer.samples[currentFrame];
	auto transform = currentSample.transform;

	//m_rigidDynamic->

	++m_currentFrame;
}

bool ActionPlayAnimation::isComplete() const
{
	return (m_frameCount == m_currentFrame);
}