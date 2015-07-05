#include <vxAnimationAspect/AnimationAspect.h>
#include <vxEngineLib/Animation.h>

struct Animation : public vx::Animation
{

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
		auto layerCount = it.layerCount;
		auto layers = it.layers.get();

		for (u32 i = 0; i < layerCount; ++i)
		{
			auto &layer = layers[i];

			
		}
	}
}