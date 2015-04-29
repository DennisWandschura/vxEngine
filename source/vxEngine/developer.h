#pragma once

#include <vxLib/types.h>
#include "DebugRenderSettings.h"

namespace dev
{
	extern U8 g_toggleSound;
	extern U8 g_showNavGraph;
	extern U8 g_toggleRender;
	extern U8 g_record;
	extern DebugRenderSettings g_debugRenderSettings;

	enum Channel : U8
	{
		Channel_Render = 1 << 0,
		Channel_Editor = 1 << 1,
		Channel_FileAspect = 1 << 2
	};
}