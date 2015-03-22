#pragma once

#include <vxLib/types.h>

class ImageBindingManager
{
	static const U8 s_maxBindings = 255;

	struct Binding
	{
		U32 unit{0};
		U32 id{0};
		U32 access{0}; 
		U32 format{0};
	};

	static Binding s_bindings[s_maxBindings];

	ImageBindingManager();

public:
	static void bind(U32 unit, U32 id, U32 level, U8 layered, U32 layer, U32 access, U32 format);
};