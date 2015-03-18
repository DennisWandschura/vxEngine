#ifndef __LIBRARIES_H
#define __LIBRARIES_H
#pragma once

#ifdef _DEBUG
#ifdef _VX_GL_45
#pragma comment(lib, "vxLib_d.lib")
#else
#pragma comment(lib, "vxLib_gl43_d.lib")
#endif
#pragma comment(lib, "yaml-cpp_d.lib")
#pragma comment(lib, "PhysX3ExtensionsDEBUG.lib")

#pragma comment(lib, "squirrel_d.lib")
#pragma comment(lib, "sqstdlib_d.lib")

#ifdef _VX_NOAUDIO
#else
#pragma comment(lib, "libvorbis_static_d.lib")
#pragma comment(lib, "libvorbisfile_static_d.lib")
#pragma comment(lib, "libogg_static_d.lib")
#pragma comment(lib, "OpenAL32.lib")
#pragma comment(lib, "vxAudio_d.lib")
#endif
#else

#pragma comment(lib, "vxLib.lib")
#pragma comment(lib, "yaml-cpp.lib")
#pragma comment(lib, "squirrel.lib")
#pragma comment(lib, "sqstdlib.lib")

#ifdef _PHYSX_CHECKED
#pragma comment(lib, "PhysX3ExtensionsCHECKED.lib")
#else
//#pragma comment(lib, "PhysX3Extensions.lib")
#endif

#ifdef _VX_NOAUDIO
#else
#pragma comment(lib, "libvorbis_static.lib")
#pragma comment(lib, "libvorbisfile_static.lib")
#pragma comment(lib, "libogg_static.lib")
#pragma comment(lib, "OpenAL32.lib")
#pragma comment(lib, "vxAudio.lib")
#endif

#endif

#pragma comment(lib, "cudart.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "cuda.lib")

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

#if _VX_IPCM
#pragma comment(lib,"Intelpcm_x64.lib")
#endif

#endif