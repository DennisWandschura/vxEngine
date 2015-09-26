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

class FileAspect;
struct EngineConfig;
class ResourceAspectInterface;
class FileAspectInterface;
class Logfile;
class SmallObjAllocator;
class CpuProfiler;

namespace vx
{
	class MessageManager;
	class Window;
	class StackAllocator;
	class TaskManager;
}

typedef void(*AbortSignalHandlerFun)(int);

#include <vxEngineLib/MessageListener.h>
#include <vxLib/math/matrix.h>
#include <string>
#include <vxEngineLib/Graphics/RenderUpdateTask.h>

struct RenderAspectDescription
{
	const std::string &dataDir;
	union
	{
		void* hwnd;
		const vx::Window* window;
	};
	const void* tmpHwnd;
	vx::StackAllocator* pAllocator;
	const EngineConfig* settings;
	Logfile* errorlog;
	CpuProfiler* cpuProfiler;
	ResourceAspectInterface* resourceAspect;
	vx::MessageManager* msgManager;
	vx::TaskManager* taskManager;
	SmallObjAllocator* smallObjAllocatorMainThread;
};

enum class RenderAspectInitializeError : s32 {OK = 0, ERROR_CONTEXT = -1, ERROR_OUT_OF_MEMORY = -2, ERROR_SHADER = -3, UNKOWN_ERROR = -4};

class RenderAspectInterface : public vx::MessageListener
{
	bool setSignalHandler(AbortSignalHandlerFun signalHandlerFn);

protected:
	virtual RenderAspectInitializeError initializeImpl(const RenderAspectDescription &desc) = 0;

public:
	virtual ~RenderAspectInterface() {}

	RenderAspectInitializeError initialize(const RenderAspectDescription &desc, AbortSignalHandlerFun signalHandlerFn)
	{
		if (!setSignalHandler(signalHandlerFn))
			return RenderAspectInitializeError::UNKOWN_ERROR;

		return initializeImpl(desc);
	}

	virtual void shutdown(void* hwnd) = 0;

	virtual bool initializeProfiler(Logfile* errorlog) = 0;

	virtual void makeCurrent(bool b) = 0;

	virtual void queueUpdate(RenderUpdateTaskType type, const u8* data, u32 dataSize) = 0;
	virtual void queueUpdateCamera(const RenderUpdateCameraData &data) = 0;

	virtual void update() = 0;

	virtual void updateProfiler(f32 dt) = 0;

	virtual void buildCommands() = 0;
	virtual void submitCommands() = 0;
	virtual void swapBuffers() = 0;
	virtual void wait() = 0;

	virtual void handleMessage(const vx::Message &evt) override = 0;

	virtual void keyPressed(u16 key) = 0;

	virtual void getProjectionMatrix(vx::mat4* m) const = 0;

	virtual void getTotalVRam(u32* totalVram) const = 0;
	virtual void getTotalAvailableVRam(u32* totalAvailableVram) const = 0;
	virtual void getAvailableVRam(u32* availableVram) const = 0;
};

typedef RenderAspectInterface* (*CreateRenderAspectFunction)();
typedef void(*DestroyRenderAspectFunction)(RenderAspectInterface *p);