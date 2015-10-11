#pragma once

/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

class CpuProfiler;

#include <vxlib/math/vector.h>
#include <vxEngineLib/MessageListener.h>
#include <vxEngineLib/ResourceAspectInterface.h>

class AudioAspectInterface : public vx::MessageListener
{
public:
	virtual ~AudioAspectInterface() {}

	virtual bool initialize(ResourceAspectInterface* resourceAspect) = 0;
	virtual void shutdown() = 0;

	virtual void update(f32 dt, CpuProfiler* profiler) = 0;

	virtual void setMasterVolume(f32 volume) = 0;

	virtual void setSourcePosition(u32 src, const vx::float3 &position) = 0;

	virtual void setListenerPosition(const __m128 &position) = 0;
	virtual void setListenerRotation(const __m128 &qRotation) = 0;
};

typedef AudioAspectInterface* (*CreateAudioAspectFunctionType)();