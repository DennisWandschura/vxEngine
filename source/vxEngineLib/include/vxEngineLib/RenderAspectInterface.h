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
class FileAspectInterface;

namespace vx
{
	class EventManager;
	class Window;
	class StackAllocator;
}

#include <vxEngineLib/EventListener.h>
#include <vxLib/math/matrix.h>
#include <string>
#include <vxEngineLib/RenderUpdateTask.h>

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
	FileAspectInterface* fileAspect;
	vx::EventManager* evtManager;
};

enum class RenderAspectInitializeError : u32 {OK, ERROR_CONTEXT, ERROR_OUT_OF_MEMORY, ERROR_SHADER};

class RenderAspectInterface : public vx::EventListener
{
public:
	virtual ~RenderAspectInterface() {}

	virtual RenderAspectInitializeError initialize(const RenderAspectDescription &desc) = 0;
	virtual void shutdown(void* hwnd) = 0;

	virtual bool initializeProfiler() = 0;

	virtual void makeCurrent(bool b) = 0;

	virtual void queueUpdateTask(const RenderUpdateTask &task) = 0;
	virtual void queueUpdateTask(const RenderUpdateTask &task, const u8* data, u32 dataSize) = 0;
	virtual void queueUpdateCamera(const RenderUpdateCameraData &data) = 0;
	virtual void update() = 0;

	virtual void updateProfiler(f32 dt) = 0;

	virtual void submitCommands() = 0;
	virtual void endFrame() = 0;

	virtual void handleEvent(const vx::Event &evt) override = 0;

	virtual void keyPressed(u16 key) = 0;

	virtual void getProjectionMatrix(vx::mat4* m) = 0;

	virtual void getTotalVRam(u32* totalVram) const = 0;
	virtual void getTotalAvailableVRam(u32* totalAvailableVram) const = 0;
	virtual void getAvailableVRam(u32* availableVram) const = 0;
};

typedef RenderAspectInterface* (*CreateRenderAspectFunction)(const RenderAspectDescription &desc, RenderAspectInitializeError* error);
typedef void(*DestroyRenderAspectFunction)(RenderAspectInterface *p);