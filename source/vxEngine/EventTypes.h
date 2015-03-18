#pragma once

#include <vxLib/types.h>

enum class EventType : U8
{
	File_Event,
	Ingame_Event,
	AI_Event,
	Editor_Event
};

enum class FileEvent : U16
{
	// arg1 contains ptr to scene, arg2 contains sid of filename
	Scene_Loaded,
	// arg1 contains sid to file, arg2 contains ptr
	Texture_Loaded,
	// arg1 contains sid to file, arg2 userdata
	Material_Loaded,
	// arg1 contains sid to file, arg2 userdata
	Mesh_Loaded
};