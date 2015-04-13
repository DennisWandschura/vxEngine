#pragma once

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
#include <vxLib/gl/Buffer.h>
#include <vxLib/gl/VertexArray.h>
#include <vxLib/Container/sorted_array.h>
#include <vxLib/StringID.h>
#include <vector>
#if VX_IPCM
#include <cpucounters.h>
#endif

class Profiler2
{
	static const U8 s_numFramesDelay = 3;
	static const U8 s_maxCharacters = 31u;
	static const U8 s_markersPerFrame = 30u;
	static const U8 s_markersPerCpuThread = s_markersPerFrame;
	static const U8 s_markersGpu = s_markersPerFrame * s_numFramesDelay;

	static const U8 s_maxGpuStringSize = 64;
	static const U32 s_maxGpuCharacters = s_maxGpuStringSize * s_markersPerFrame;
	static const U32 s_maxVertices = s_maxGpuCharacters * 4u;
	static const U32 s_maxIndices = s_maxGpuCharacters * 6u;

	static I64 s_cpuFrequency;
	static vx::float2 s_position;

	struct Vertex
	{
		vx::float3 inputPosition;
		vx::float3 inputTexCoords;
		vx::float4 inputColor;
	};

	struct Marker
	{
		I64 start{0};
		I64 end{0};
		char name[s_maxCharacters];
		U8 layer{0};

		Marker()
		{
			name[0] = '\0';
		}
	};

	struct GpuMarker : public Marker
	{
		U32 id_query_start{ 0 };
		U32 id_query_end{ 0 };
		I64 frame{-1};
	};

	struct CpuThreadInfo
	{
		Marker markers[s_markersPerCpuThread];
		U8 currentWriteId;
		U8 m_pushedMarkers;
	};

	struct GpuThreadInfo
	{
		GpuMarker markers[s_markersGpu];
		U8 currentWriteId{0};
		U8 m_pushedMarkers{0};
	};

	struct EntryCpu
	{
		U32 time;
		U32 timeMin{-1};
		U32 timeMax{0};
		char name[s_maxCharacters];
		U8 layer;
	};

	struct EntryGpu
	{
		U32 time;
		U32 timeMin{ -1 };
		U32 timeMax{ 0 };
		char name[s_maxCharacters];
		U8 layer;
		U32 queryStart{0};
		U32 queryEnd{0};

		EntryGpu()
		{
			name[0] = '\0';
		}
	};

	I64 m_currentFrame{ 0 };
	CpuThreadInfo m_cpuThreadInfo;
	GpuThreadInfo m_gpuThreadInfo;
	vx::sorted_array<vx::StringID64, U32> m_entriesCpuByName;
	vx::sorted_array<vx::StringID64, U32> m_entriesGpuByName;
	U16 m_entryCpuCount{0};
	U16 m_entryGpuCount{0};
	std::unique_ptr<EntryCpu[]> m_entriesCpu;
	std::unique_ptr<EntryGpu[]> m_entriesGpu;
	const vx::gl::ProgramPipeline *m_pPipeline{ nullptr };
	vx::gl::VertexArray m_vao;
	U32 m_indexCount{ 0 };
	U32 m_textureIndex{ 0 };
	const Font* m_pFont{ nullptr };
	std::unique_ptr<Vertex[]> m_pVertices{};
	vx::gl::Buffer m_ibo;
	vx::gl::Buffer m_vbo;
#if VX_IPCM
	PCM* m_pInstance{ nullptr };
#endif

	void updateBuffer(const U32 ascii_code, const vx::float2 &position_x_texSlice, const vx::uint2a &textureSize, const __m128 &invTextureSize, const __m128 &color, vx::uint2 *bufferIndex, vx::float2 *cursorPos);

	void writeBuffer(I32 strSize, const char* buffer, const vx::float2 &position_x_texSlice, const vx::uint2a &textureSize, const __m128 &vInvTexSize, const __m128 &color, vx::uint2* bufferIndex, vx::float2* cursorPos);
	void writeCpuMarkers(F32 textureSlice, const vx::uint2a &textureSize, const __m128 &vInvTexSize, vx::uint2* bufferIndex, vx::float2 *cursorPos);
	void writeGpuMarkers(F32 textureSlice, const vx::uint2a &textureSize, const __m128 &vInvTexSize, vx::uint2* bufferIndex, vx::float2 *cursorPos);

public:
	Profiler2();
	~Profiler2();

	bool initialize(const Font* pFont, const vx::gl::ProgramPipeline* pPipeline, U32 textureIndex, const vx::uint2 windowResolution, vx::StackAllocator* pAllocator);

	void update(F32 dt);

	void render();

	void frame();

	void pushGpuMarker(const char *name);
	void popGpuMarker();

	void pushCpuMarker(const char* name);
	void popCpuMarker();
};