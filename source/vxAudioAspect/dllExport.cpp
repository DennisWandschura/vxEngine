#include "dllExport.h"
#include "AudioAspect.h"

AudioAspectInterface* createAudioAspect()
{
	return new vx::AudioAspect();
}