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
#include "BufferBindingManager.h"
#include <vxGL/gl.h>

namespace gl
{
	BufferBindingManager::Binding BufferBindingManager::s_uniformBindings[s_maxBindings]{};
	BufferBindingManager::Binding BufferBindingManager::s_shaderStorageBindings[s_maxBindings]{};

	void BufferBindingManager::bindBaseUniform(u32 index, u32 bufferId)
	{
		if (index >= s_maxBindings)
			return;

		auto current = s_uniformBindings[index];
		if (current.bufferId != bufferId)
		{
			glBindBufferBase(GL_UNIFORM_BUFFER, index, bufferId);
			s_uniformBindings[index].bufferId = bufferId;
		}
	}

	void BufferBindingManager::bindBaseShaderStorage(u32 index, u32 bufferId)
	{
		if (index >= s_maxBindings)
			return;

		auto current = s_shaderStorageBindings[index];
		if (current.bufferId != bufferId)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, bufferId);
			s_shaderStorageBindings[index].bufferId = bufferId;
		}
	}
}