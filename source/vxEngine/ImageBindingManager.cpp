#include "ImageBindingManager.h"
#include <vxLib/gl/gl.h>

ImageBindingManager::Binding ImageBindingManager::s_bindings[s_maxBindings]{};

ImageBindingManager::ImageBindingManager(){}

void ImageBindingManager::bind(U32 unit, U32 id, U32 level, U8 layered, U32 layer, U32 access, U32 format)
{
	if (unit >= s_maxBindings)
		return;

	auto currentBinding = s_bindings[unit];
	if (currentBinding.id != id ||
		currentBinding.access != access ||
		currentBinding.format != format)
	{
		glBindImageTexture(unit, id, level, layered, layer, access, format);

		s_bindings[unit].id = id;
		s_bindings[unit].access = access;
		s_bindings[unit].format = format;
	}
}