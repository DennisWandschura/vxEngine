#pragma once
#if _VX_PROFILER

class Font;

namespace vx
{
	namespace gl
	{
		class ProgramPipeline;
		class StateManager;
	}
}

#include <vxLib/math/Vector.h>
#include "Buffer.h"
#include "VertexArray.h"
#include "sorted_array.h"
#include <vxLib/StringID.h>

class Profiler
{
	static const U8 s_numQueries = 10u;
	static const U8 s_numFrames = 3u;
	static const U8 s_maxCharacters = 32;
	static const U8 s_markersPerFrame = 30;
	static const U8 s_markersPerThread = s_markersPerFrame * s_numFrames;

	static const U8 s_maxCpuThreads = 1;

	static const U8 s_maxGpuStringSize = 64;
	static const U32 s_maxGpuCharacters = s_maxGpuStringSize * s_markersPerFrame * (s_maxCpuThreads + 1);
	static const U32 s_maxVertices = s_maxGpuCharacters * 4u;
	static const U32 s_maxIndices = s_maxGpuCharacters * 6u;

	static vx::int2 s_positionGpu;
	static vx::int2 s_positionCpu;
	static U8 s_update;

	static I64 s_cpuFrequency;

	struct Vertex
	{
		vx::float3 inputPosition;
		vx::float3 inputTexCoords;
		vx::float4 inputColor;
	};

	struct Entry
	{
		U32 count{ 0 };
		U32 layer{ 0 };
		F32 avgTime{ 0.0f };
		F32 maxTime{ 0.0f };
		F32 minTime{ 0.0f };
	};

	struct Marker
	{
		U64 start{ 0 };
		U64 end{ 0 };
		char name[s_maxCharacters];
		U32 layer;
		U64 frame{ 0 };

		Marker() :start(0), end(0), frame(-1){}
	};

	struct GpuMarker : public Marker
	{
		U32 id_query_start{ 0 };
		U32 id_query_end{ 0 };
	};

	struct GpuThreadInfo
	{
		GpuMarker markers[s_markersPerThread];
		I64 currentWriteId{ 0 };
		U32	currentReadId{ 0 };
		U32 nextReadId{ 0 };
		size_t nb_pushed_markers{ 0 };
	};

	struct CpuThreadInfo
	{
		Marker markers[s_markersPerThread];
		I64 currentWriteId{ 0 };
		U32	currentReadId{ 0 };
		U32 nextReadId{ 0 };
		size_t nb_pushed_markers{ 0 };
	};

	GpuThreadInfo m_gpuThreadInfo;
	CpuThreadInfo m_cpuThreadInfo;
	U64 m_currentFrame{ 0 };
	const vx::gl::ProgramPipeline *m_pPipeline{ nullptr };
	vx::gl::VertexArray m_vao;
	U32 m_indexCount{ 0 };
	U32 m_textureIndex{ 0 };
	vx::sorted_array<vx::StringID64, Entry> m_entries;
	const Font* m_pFont{ nullptr };
	std::unique_ptr<Vertex[]> m_pVertices{};
	vx::gl::Buffer m_ibo;
	vx::gl::Buffer m_vbo;

	void updateCpuThreadInfo(I64 displayed_frame, U8* cpuStackIndex, U8(&cpuStack)[s_markersPerThread]);
	void updateGpuThreadInfo(I64 displayed_frame, U8* gpuStackIndex, U8(&gpuStack)[s_markersPerThread]);

	void updateBuffer(const U32 ascii_code, const vx::float2 position_x_texSlice, vx::float2 &cursorPos, const vx::uint2 textureSize, const __m128 &invTextureSize, const __m128 &color, vx::uint2 &index);

	void updateEntry(const vx::StringID64 sid, const GpuMarker &marker);

	void writeGpuMarkers(vx::float2 cursorPos, F32 textureSlice, U32 stackSize, const U8(&readStack)[s_markersPerThread], const vx::uint2 &textureSize, const __m128 &vInvTexSize, vx::uint2* bufferIndex);
	void writeCpuMarkers(vx::float2 cursorPos, F32 textureSlice, U32 stackSize, const U8(&readStack)[s_markersPerThread], const vx::uint2 &textureSize, const __m128 &vInvTexSize, vx::uint2* bufferIndex);

public:
	Profiler();

	void initialize(const Font* pFont, const vx::gl::ProgramPipeline* pPipeline, U32 textureIndex, const vx::uint2 windowResolution, vx::StackAllocator* pAllocator);

	void frame();

	void update();

	void render(vx::gl::StateManager &stateManager);

	void pushGpuMarker(const char *name);
	void popGpuMarker();

	void pushCpuMarker(const char *name);
	void popCpuMarker();
};
#endif