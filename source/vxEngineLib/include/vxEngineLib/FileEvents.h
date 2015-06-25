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

#include <vxLib/types.h>

namespace vx
{
	enum class FileEvent : u16
	{
		// arg1 contains sid of filename, arg2 contains ptr to scene
		Scene_Loaded,
		EditorScene_Loaded,
		// arg1 contains sid to file, arg2 contains ptr
		Texture_Loaded,
		// arg1 contains sid to file, arg2 userdata
		Material_Loaded,
		// arg1 contains sid to file, arg2 userdata
		Mesh_Loaded,
		Wav_Loaded,
		Animation_Loaded,

		Scene_Existing,
		Texture_Existing,
		Material_Existing,
		Mesh_Existing
	};
}