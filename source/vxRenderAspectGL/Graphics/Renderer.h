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

namespace gl
{
	class ObjectManager;
}

namespace vx
{
	class StackAllocator;

	namespace gl
	{
		class  ShaderManager;
	}
}

struct EngineConfig;
class GpuProfiler;

#include <vector>
#include <vxLib/types.h>

namespace Graphics
{
	class CommandList;

	class Renderer
	{
	protected:
		static vx::gl::ShaderManager* s_shaderManager;
		static gl::ObjectManager* s_objectManager;
		static const EngineConfig* s_settings;
		static GpuProfiler* s_gpuProfiler;

	public:
		virtual ~Renderer(){}

		virtual void initialize(vx::StackAllocator* scratchAllocator, const void* p) = 0;
		virtual void shutdown() = 0;

		virtual void getCommandList(CommandList* cmdList) = 0;

		virtual void clearData() = 0;
		virtual void bindBuffers() = 0;

		static void provide(vx::gl::ShaderManager* shaderManager, gl::ObjectManager* objectManager, const EngineConfig* settings, GpuProfiler* gpuProfiler);
	};
}