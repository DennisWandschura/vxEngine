#pragma once

#include <vxlib/types.h>
#include <vxEngineLib/MessageListener.h>

class AudioAspectInterface : public vx::MessageListener
{
public:
	virtual ~AudioAspectInterface() {}

	virtual bool initialize() = 0;
	virtual void shutdown() = 0;

	virtual void update(f32 dt) = 0;
};

typedef AudioAspectInterface* (*CreateAudioAspectFunctionType)();