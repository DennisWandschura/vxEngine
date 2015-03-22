#pragma once

#include <vxLib/types.h>

class BufferBindingManager
{
	static const U8 s_maxBindings = 255;

	struct Binding
	{
		U32 index;
		U32 bufferId;
	};

	static Binding s_uniformBindings[s_maxBindings];
	static Binding s_shaderStorageBindings[s_maxBindings];

public:
	static void bindBaseUniform(U32 index, U32 bufferId);
	static void bindBaseShaderStorage(U32 index, U32 bufferId);
};