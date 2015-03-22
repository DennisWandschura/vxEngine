#include "BufferBindingManager.h"
#include <vxLib/gl/gl.h>

BufferBindingManager::Binding BufferBindingManager::s_uniformBindings[s_maxBindings]{};
BufferBindingManager::Binding BufferBindingManager::s_shaderStorageBindings[s_maxBindings]{};

void BufferBindingManager::bindBaseUniform(U32 index, U32 bufferId)
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

void BufferBindingManager::bindBaseShaderStorage(U32 index, U32 bufferId)
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