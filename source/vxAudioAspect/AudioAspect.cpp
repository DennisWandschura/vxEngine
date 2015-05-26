#include "vxAudio/AudioAspect.h"

bool AudioAspect::initialize()
{
	return m_audioManager.init();
}

void AudioAspect::shutdown()
{
	m_audioManager.shutdown();
}

void AudioAspect::update()
{

}

void AudioAspect::handleEvent(const vx::Event &evt)
{

}