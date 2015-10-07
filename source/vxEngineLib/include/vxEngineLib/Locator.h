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
	class MessageManager;
}
class PhysicsAspect;
class ResourceAspect;
class RenderAspectInterface;
class AudioAspectInterface;

class Locator
{
	static vx::MessageManager* s_pEventManager;
	static PhysicsAspect* s_pPhysicsAspect;
	static ResourceAspect* s_pResourceAspect;
	static RenderAspectInterface* s_pRenderAspect;
	static AudioAspectInterface* s_audioAspectInterface;

public:
	static void provide(vx::MessageManager* p);
	static vx::MessageManager* getMessageManager();

	static void provide(PhysicsAspect* p);
	static PhysicsAspect* getPhysicsAspect();

	static void provide(ResourceAspect* p);
	static ResourceAspect* getResourceAspect();

	static void provide(RenderAspectInterface* p);
	static RenderAspectInterface* getRenderAspect();

	static void provide(AudioAspectInterface* p);
	static AudioAspectInterface* getAudioAspect();

	static void reset();
};