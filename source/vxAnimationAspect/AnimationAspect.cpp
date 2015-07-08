#include <vxAnimationAspect/AnimationAspect.h>
#include <vxEngineLib/Animation.h>
#include <vxEngineLib/Entity.h>
#include <vxEngineLib/Locator.h>
#include <vxResourceAspect/FileAspect.h>

struct Animation
{
	vx::Transform currentTarget;
	vx::Transform currentStatus;
	Reference<vx::Animation> animation;
	f32 timeLeft;
	u32 currentFrame;
};

AnimationAspect::AnimationAspect()
	:m_animations()
{

}

AnimationAspect::~AnimationAspect()
{

}

void AnimationAspect::handleEvent(const vx::Event &evt)
{

}

void AnimationAspect::update(const f32 dt)
{
	for (auto &it : m_animations)
	{
	}
}

void AnimationAspect::addAnimation(StaticEntityAnimated* entity)
{
	auto fileAspect = Locator::getFileAspect();
	auto animation = fileAspect->getAnimation(entity->animSid);
	VX_ASSERT(animation.isValid());

	auto frameCount = (*animation).layers[0].frameCount;
	auto frameRate = (*animation).layers[0].frameRate;

	Animation anim;
	anim.animation = animation;
	anim.timeLeft = 0.0f;
	anim.currentFrame = 0;
}