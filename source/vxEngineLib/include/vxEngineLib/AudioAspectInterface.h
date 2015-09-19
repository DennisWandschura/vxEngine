#pragma once

#include <vxlib/types.h>
#include <vxEngineLib/MessageListener.h>
#include <vxEngineLib/ResourceAspectInterface.h>

class AudioAspectInterface : public vx::MessageListener
{
public:
	virtual ~AudioAspectInterface() {}

	virtual bool initialize(ResourceAspectInterface* resourceAspect) = 0;
	virtual void shutdown() = 0;

	virtual void update(f32 dt) = 0;

	virtual void setMasterVolume(f32 volume) = 0;
};

typedef AudioAspectInterface* (*CreateAudioAspectFunctionType)();