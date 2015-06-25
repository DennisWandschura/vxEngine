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

#if defined(_DEBUG_STATIC_BUILD)

#pragma comment(lib, "vxLib_sd.lib")
#pragma comment(lib, "vxGL_sd.lib")
#pragma comment(lib, "PhysX3ExtensionsDEBUG.lib")
#pragma comment(lib, "vxRenderAspectGL_d.lib")

#if _VX_EDITOR
#pragma comment(lib, "ResourceAspect_editor_d.lib")
#pragma comment(lib, "vxEngineLib_editor_d.lib")
#pragma comment(lib, "libfbxsdk-mt.lib")
#else
#pragma comment(lib, "vxResourceAspect_d.lib")
#pragma comment(lib, "vxEngineLib_d.lib")
#endif

#ifdef _VX_NOAUDIO
#else
#pragma comment(lib, "libvorbis_static_d.lib")
#pragma comment(lib, "libvorbisfile_static_d.lib")
#pragma comment(lib, "libogg_static_d.lib")
#pragma comment(lib, "OpenAL32.lib")
#pragma comment(lib, "vxAudioAspect_d.lib")
#endif

#elif defined(_RELEASE_STATIC_BUILD)
#pragma comment(lib, "vxLib_s.lib")
#pragma comment(lib, "vxGL_s.lib")
#pragma comment(lib, "vxEngineLib.lib")
#pragma comment(lib, "ResourceAspect.lib")

#ifdef _VX_NOAUDIO
#else
#pragma comment(lib, "libvorbis_static.lib")
#pragma comment(lib, "libvorbisfile_static.lib")
#pragma comment(lib, "libogg_static.lib")
#pragma comment(lib, "OpenAL32.lib")
#pragma comment(lib, "vxAudioAspect.lib")
#endif

#ifdef _PHYSX_CHECKED
#pragma comment(lib, "PhysX3ExtensionsCHECKED.lib")
#else
#endif

#endif

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "OpenCL.lib")
#pragma comment(lib, "Shlwapi.lib")

#ifdef _PHYSX_CHECKED
#pragma comment(lib, "PhysX3CharacterKinematicCHECKED_x64.lib")
#pragma comment(lib, "PhysX3CHECKED_x64.lib")
#pragma comment(lib, "PhysX3CommonCHECKED_x64.lib")
#pragma comment(lib, "PhysX3CookingCHECKED_x64.lib")
#else
#pragma comment(lib, "PhysX3CharacterKinematic_x64.lib")
#pragma comment(lib, "PhysX3_x64.lib")
#pragma comment(lib, "PhysX3Common_x64.lib")
#pragma comment(lib, "PhysX3Cooking_x64.lib")
#endif