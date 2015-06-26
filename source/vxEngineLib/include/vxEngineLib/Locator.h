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

namespace vx
{
	class EventManager;
}
class PhysicsAspect;
class FileAspect;
class RenderAspectInterface;

class Locator
{
	static vx::EventManager* s_pEventManager;
	static PhysicsAspect* s_pPhysicsAspect;
	static FileAspect* s_pFileAspect;
	static RenderAspectInterface* s_pRenderAspect;

public:
	static void provide(vx::EventManager* p);
	static vx::EventManager* getEventManager();

	static void provide(PhysicsAspect* p);
	static PhysicsAspect* getPhysicsAspect();

	static void provide(FileAspect* p);
	static FileAspect* getFileAspect();

	static void provide(RenderAspectInterface* p);
	static RenderAspectInterface* getRenderAspect();

	static void reset();
};