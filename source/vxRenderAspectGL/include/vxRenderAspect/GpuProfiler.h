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

#include <vxLib/math/Vector.h>
#include <vxGL/Buffer.h>
#include <vxGL/VertexArray.h>
#include <vxLib/Container/sorted_array.h>
#include <vxLib/StringID.h>
#include <vector>

class GpuProfiler
{
	static const u8 s_numFramesDelay = 3;
	static const u8 s_maxCharacters = 31u;
	static const u8 s_markersPerFrame = 30u;
	static const u8 s_markersPerCpuThread = s_markersPerFrame;
	static const u8 s_markersGpu = s_markersPerFrame * s_numFramesDelay;

	static const u8 s_maxGpuStringSize = 64;
	static const u32 s_maxGpuCharacters = s_maxGpuStringSize * s_markersPerFrame;
	static const u32 s_maxVertices = s_maxGpuCharacters * 4u;
	static const u32 s_maxIndices = s_maxGpuCharacters * 6u;

	static s64 s_cpuFrequency;
	static vx::float2 s_position;

	struct Vertex
	{
		vx::float3 inputPosition;
		vx::float3 inputTexCoords;
		vx::float4 inputColor;
	};

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

	struct EntryGpu
	{
		u32 time{0};
		u32 timeMin{ 0xffffffff };
		u32 timeMax{ 0 };
		char name[s_maxCharacters];
		u8 layer{0};
		u32 queryStart{ 0 };
		u32 queryEnd{ 0 };

		EntryGpu()
		{
			name[0] = '\0';
		}
	};

	s64 m_currentFrame{ 0 };
	GpuThreadInfo m_gpuThreadInfo;
	vx::sorted_array<vx::StringID, u32> m_entriesGpuByName;
	u16 m_entryGpuCount{ 0 };
	std::unique_ptr<EntryGpu[]> m_entriesGpu;
	const vx::gl::ProgramPipeline *m_pPipeline{ nullptr };
	vx::gl::VertexArray m_vao;
	u32 m_indexCount{ 0 };
	u32 m_textureIndex{ 0 };
	const Font* m_pFont{ nullptr };
	std::unique_ptr<Vertex[]> m_pVertices{};
	vx::gl::Buffer m_ibo;
	vx::gl::Buffer m_vbo;

	void updateBuffer(const u32 ascii_code, const vx::float2 &position_x_texSlice, const vx::uint2a &textureSize, const __m128 &invTextureSize, const __m128 &color, vx::uint2 *bufferIndex, vx::float2 *cursorPos);

	void writeBuffer(s32 strSize, const char* buffer, const vx::float2 &position_x_texSlice, const vx::uint2a &textureSize, const __m128 &vInvTexSize, const __m128 &color, vx::uint2* bufferIndex, vx::float2* cursorPos);
	void writeGpuMarkers(f32 textureSlice, const vx::uint2a &textureSize, const __m128 &vInvTexSize, vx::uint2* bufferIndex, vx::float2 *cursorPos);

public:
	GpuProfiler();
	~GpuProfiler();

	bool initialize(const Font* pFont, const vx::gl::ProgramPipeline* pPipeline, u32 textureIndex, const vx::uint2 &windowResolution, vx::StackAllocator* pAllocator);

	void update(f32 dt);

	void render();

	void frame();

	void pushGpuMarker(const char *name);
	void popGpuMarker();
};