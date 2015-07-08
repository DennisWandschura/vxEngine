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

class Font;

namespace vx
{
	namespace gl
	{
		class ProgramPipeline;
		class StateManager;
	}

	class StackAllocator;
}

namespace Graphics
{
	class TextRenderer;
}

#include <vxLib/math/Vector.h>
#include <vxLib/Container/sorted_array.h>
#include <vxLib/StringID.h>
#include <vector>
#include <memory>

class GpuProfiler
{
	static const u8 s_numFramesDelay = 3;
	static const u8 s_maxCharacters = 31u;
	static const u8 s_markersPerFrame = 30u;
	static const u8 s_markersGpu = s_markersPerFrame * s_numFramesDelay;

	static const u8 s_maxGpuStringSize = 64;
	static const u32 s_maxGpuCharacters = s_maxGpuStringSize * s_markersPerFrame;

	struct Marker
	{
		s64 start{ 0 };
		s64 end{ 0 };
		char name[s_maxCharacters];
		u8 layer{ 0 };

		Marker()
		{
			name[0] = '\0';
		}
	};

	struct GpuMarker : public Marker
	{
		u32 id_query_start{ 0 };
		u32 id_query_end{ 0 };
		s64 frame{ -1 };
	};

	struct GpuThreadInfo
	{
		GpuMarker markers[s_markersGpu];
		u8 currentWriteId{ 0 };
		u8 m_pushedMarkers{ 0 };
	};

	struct EntryGpu;
	

	s64 m_currentFrame;
	GpuThreadInfo m_gpuThreadInfo;
	vx::sorted_array<vx::StringID, u32> m_entriesGpuByName;
	u16 m_entryGpuCount;
	std::unique_ptr<EntryGpu[]> m_entriesGpu;
	Graphics::TextRenderer* m_textRenderer;
	vx::float2 m_position;

public:
	GpuProfiler();
	~GpuProfiler();

	bool initialize(const vx::float2 &position, Graphics::TextRenderer* textRenderer, vx::StackAllocator* pAllocator);

	void update();

	void frame();

	void pushGpuMarker(const char *name);
	void popGpuMarker();
};