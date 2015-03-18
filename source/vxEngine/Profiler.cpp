#if _VX_PROFILER
#ifdef false
#include "Profiler.h"
#include <vxLib\gl\gl.h>
#include <cstring>
#include <cstdio>
#include "Font.h"
#include <vxLib\gl\ProgramPipeline.h>
#include <vxLib/gl/StateManager.h>
#include <Windows.h>

vx::int2 Profiler::s_positionGpu{ 0, 0 };
vx::int2 Profiler::s_positionCpu{ 0, 0 };
I64 Profiler::s_cpuFrequency{ 1 };
U8 Profiler::s_update{0};

inline void	incrementCycle(I32* pval, size_t array_size)
{
	I32 val = *pval;
	++val;

	if (val >= (I32)(array_size))
		val = 0;

	*pval = val;
}

inline void	incrementCycle(I64* pval, size_t array_size)
{
	I64 val = *pval;
	++val;

	if (val >= (I64)(array_size))
		val = 0;

	*pval = val;
}

inline void	incrementCycle(U32* pval, size_t array_size)
{
	I64 val = *pval;
	++val;

	if (val >= (I64)(array_size))
		val = 0;

	*pval = val;
}

inline void	decrementCycle(U32* pval, size_t array_size)
{
	I64 val = *pval;
	--val;

	if (val < 0)
		val = (I64)(array_size - 1);

	*pval = val;
}

inline void	decrementCycle(I64* pval, size_t array_size)
{
	I64 val = *pval;
	--val;

	if (val < 0)
		val = (I64)(array_size - 1);

	*pval = val;
}

Profiler::Profiler()
	:m_gpuThreadInfo(),
	m_cpuThreadInfo()
{
	LARGE_INTEGER frq;
	QueryPerformanceFrequency(&frq);
	s_cpuFrequency = frq.QuadPart;
}

void Profiler::initialize(const Font* pFont, const vx::gl::ProgramPipeline* pPipeline, U32 textureIndex, const vx::uint2 windowResolution, vx::StackAllocator* pAllocator)
{
	m_pPipeline = pPipeline;
	m_pFont = pFont;
	m_textureIndex = textureIndex;

	m_entries = vx::sorted_array<vx::StringID64, Entry>(s_markersPerThread, pAllocator);

	s_positionGpu.x = -(windowResolution.x / 2) + 25;
	s_positionGpu.y = (windowResolution.y / 2) - 25;

	m_pVertices = std::make_unique<Vertex[]>(s_maxVertices);

	for (auto i = 0u; i < s_markersPerThread; ++i)
	{
		glCreateQueries(GL_TIMESTAMP, 1, &m_gpuThreadInfo.markers[i].id_query_start);
		glCreateQueries(GL_TIMESTAMP, 1, &m_gpuThreadInfo.markers[i].id_query_end);
	}

	vx::gl::BufferDescription vboDesc;
	vboDesc.bufferType = vx::gl::BufferType::Array_Buffer;
	vboDesc.flags = vx::gl::BufferStorageFlags::Write;
	vboDesc.immutable = 1;
	vboDesc.size = sizeof(Vertex) * s_maxVertices;
	m_vbo.create(vboDesc);

	std::unique_ptr<U32[]> pIndices = std::make_unique<U32[]>(s_maxIndices);
	for (U32 i = 0, j = 0; i < s_maxIndices; i += 6, j += 4)
	{
		pIndices[i] = j;
		pIndices[i + 1] = j + 1;
		pIndices[i + 2] = j + 2;

		pIndices[i + 3] = j + 2;
		pIndices[i + 4] = j + 3;
		pIndices[i + 5] = j;
	}

	vx::gl::BufferDescription iboDesc;
	iboDesc.bufferType = vx::gl::BufferType::Element_Array_Buffer;
	iboDesc.flags = vx::gl::BufferStorageFlags::None;
	iboDesc.immutable = 1;
	iboDesc.size = sizeof(U32) * s_maxIndices;
	iboDesc.pData = pIndices.get();
	m_ibo.create(iboDesc);

	m_vao.create();
	m_vao.bindIndexBuffer(m_ibo);

	// vx::float3 inputPosition;
	m_vao.enableArrayAttrib(0);
	m_vao.arrayAttribFormatF(0, 3, 0, 0);
	m_vao.arrayAttribBinding(0, 0);

	// vx::float3 inputTexCoords;
	m_vao.enableArrayAttrib(1);
	m_vao.arrayAttribFormatF(1, 3, 0, sizeof(vx::float3));
	m_vao.arrayAttribBinding(1, 0);

	// vx::float4 inputColor;
	m_vao.enableArrayAttrib(2);
	m_vao.arrayAttribFormatF(2, 4, 0, sizeof(vx::float3) * 2);
	m_vao.arrayAttribBinding(2, 0);

	m_vao.bindVertexBuffer(m_vbo, 0, 0, sizeof(Vertex));
}

void Profiler::frame()
{
	++m_currentFrame;

	m_gpuThreadInfo.currentReadId = m_gpuThreadInfo.nextReadId;
	m_cpuThreadInfo.currentReadId = m_cpuThreadInfo.nextReadId;
}

void Profiler::updateEntry(const vx::StringID64 sid, const GpuMarker &marker)
{
	auto it = m_entries.find(sid);
	if (it == m_entries.end())
	{
		it = m_entries.insert(sid, Entry());
		VX_ASSERT(it != m_entries.end(), "Out of memory");
	}


	auto time = double(marker.end - marker.start) * 1.0e-6;

	if (it->count == 100)
	{
		//it->avgTime = 0.0f;
		it->maxTime = 0.0f;
		it->count = 0;
	}

	it->maxTime = std::max(it->maxTime, (float)time);

	it->avgTime = (time + it->avgTime * it->count) / (it->count + 1);
	it->layer = marker.layer;
	++it->count;
}

void Profiler::writeGpuMarkers(vx::float2 cursorPos, F32 textureSlice, U32 stackSize, const U8(&readStack)[s_markersPerThread], const vx::uint2 &textureSize, const __m128 &vInvTexSize, vx::uint2* bufferIndex)
{
	const __m128 vColor = { 0.0f, 0.8f, 0.0f, 1 };
	char buffer[s_maxGpuStringSize];

	vx::float2 position_x_texSlice = { cursorPos.x, textureSlice };
	for (auto j = 0u; j < stackSize; ++j)
	{
		auto index = readStack[j];
		//auto sid = sidStack[i];
		auto &marker = m_gpuThreadInfo.markers[index];

		//auto itEntry = m_entries.find(sid);

		int strSize = sprintf_s(buffer, "%s %.6f ms\n", marker.name, double(marker.end - marker.start) * 1.0e-6);
		//int strSize = sprintf_s(buffer, "%s avg: %.6f ms, max: %.6f\n", marker.name, itEntry->avgTime, itEntry->maxTime);

		cursorPos.x += marker.layer * 25.0f;

		for (I32 i = 0; i < strSize; ++i)
		{
			char ascii_code = buffer[i];

			updateBuffer(ascii_code, position_x_texSlice, cursorPos, textureSize, vInvTexSize, vColor, *bufferIndex);
		}
	}

	Vertex* pVertices = reinterpret_cast<Vertex*>(m_vbo.map(vx::gl::Map::Write_Only));
	memcpy(pVertices, m_pVertices.get(), sizeof(Vertex) * bufferIndex->x);
	m_vbo.unmap();
}

void Profiler::writeCpuMarkers(vx::float2 cursorPos, F32 textureSlice, U32 stackSize, const U8(&readStack)[s_markersPerThread], const vx::uint2 &textureSize, const __m128 &vInvTexSize, vx::uint2* bufferIndex)
{
	const __m128 vColor = { 0.0f, 0.8f, 0.0f, 1 };
	char buffer[s_maxGpuStringSize];

	vx::float2 position_x_texSlice = { cursorPos.x, textureSlice };
	for (auto j = 0u; j < stackSize; ++j)
	{
		auto index = readStack[j];
		//auto sid = sidStack[i];
		auto &marker = m_cpuThreadInfo.markers[index];

		//auto itEntry = m_entries.find(sid);

		I64 ElapsedMicroseconds = marker.end - marker.start;
		ElapsedMicroseconds *= 1000000;
		ElapsedMicroseconds /= s_cpuFrequency;

		double miliseconds = 0.001 * ElapsedMicroseconds;
		int strSize = sprintf_s(buffer, "%s %.6f ms\n", marker.name, miliseconds);

		cursorPos.x += marker.layer * 25.0f;

		for (I32 i = 0; i < strSize; ++i)
		{
			char ascii_code = buffer[i];

			updateBuffer(ascii_code, position_x_texSlice, cursorPos, textureSize, vInvTexSize, vColor, *bufferIndex);
		}
	}

	Vertex* pVertices = reinterpret_cast<Vertex*>(m_vbo.map(vx::gl::Map::Write_Only));
	memcpy(pVertices, m_pVertices.get(), sizeof(Vertex) * bufferIndex->x);
	m_vbo.unmap();
}

void Profiler::updateCpuThreadInfo(I64 displayed_frame, U8* cpuStackIndex, U8(&cpuStack)[s_markersPerThread])
{
	U32 read_id = m_cpuThreadInfo.currentReadId;

	while (m_cpuThreadInfo.markers[read_id].frame == displayed_frame)
	{
		auto &marker = m_cpuThreadInfo.markers[read_id];

		if (marker.start != 0 ||
			marker.end != 0)
		{
			cpuStack[*cpuStackIndex] = read_id;

			++(*cpuStackIndex);
		}

		incrementCycle(&read_id, s_markersPerThread);
	}

	m_cpuThreadInfo.nextReadId = read_id;
}

void Profiler::updateGpuThreadInfo(I64 displayed_frame, U8* gpuStackIndex, U8(&gpuStack)[s_markersPerThread])
{
	U32 read_id = m_gpuThreadInfo.currentReadId;

	while (m_gpuThreadInfo.markers[read_id].frame == displayed_frame)
	{
		auto &marker = m_gpuThreadInfo.markers[read_id];

		bool ok = true;

		if (marker.id_query_start == 0 ||
			marker.id_query_end == 0)
			ok = false;

		if (ok)
		{
			GLint start_ok = 0;
			GLint end_ok = 0;
			glGetQueryObjectiv(marker.id_query_start, GL_QUERY_RESULT_AVAILABLE, &start_ok);
			glGetQueryObjectiv(marker.id_query_end, GL_QUERY_RESULT_AVAILABLE, &end_ok);
			ok = (bool)(start_ok && end_ok);
		}

		if (ok)
		{
			glGetQueryObjectui64v(marker.id_query_start, GL_QUERY_RESULT, &marker.start);
			glGetQueryObjectui64v(marker.id_query_end, GL_QUERY_RESULT, &marker.end);

			//vx::StringID64 sid(marker.name);

			//updateEntry(sid, marker);
			//sidStack[gpuStackIndex] = sid;
			gpuStack[*gpuStackIndex] = read_id;

			++(*gpuStackIndex);


			//	printf("%s: %f ms\n", marker.name, double(marker.end - marker.start) * 1.0e-6);
		}

		incrementCycle(&read_id, s_markersPerThread);
	}

	m_gpuThreadInfo.nextReadId = read_id;
}

void Profiler::update()
{
	I64 displayed_frame = m_currentFrame - s_numFrames - 1;
	if (displayed_frame < 0) // don't draw anything during the first frames
		return;

	U8 gpuStack[s_markersPerThread];
	//vx::StringID64 sidStack[s_markersPerThread];
	U8 gpuStackIndex = 0;

	updateGpuThreadInfo(displayed_frame, &gpuStackIndex, gpuStack);

	U8 cpuStack[s_markersPerThread];
	U8 cpuStackIndex = 0;
	updateCpuThreadInfo(displayed_frame, &cpuStackIndex, cpuStack);

	F32 textureSlice = m_pFont->getTextureEntry().getSlice();
	auto textureSize = m_pFont->getTextureEntry().getTextureSize();
	vx::float2 invTextureSize = { 1.0f / textureSize.x, 1.0f / textureSize.y };
	__m128 vInvTexSize = vx::loadFloat(&invTextureSize);
	vInvTexSize = _mm_shuffle_ps(vInvTexSize, vInvTexSize, _MM_SHUFFLE(1, 0, 1, 0));

	vx::uint2 bufferIndex = { 0, 0 };
	writeGpuMarkers(s_positionGpu, textureSlice, gpuStackIndex, gpuStack, textureSize, vInvTexSize, &bufferIndex);

	writeCpuMarkers(s_positionCpu, textureSlice, cpuStackIndex, cpuStack, textureSize, vInvTexSize, &bufferIndex);

	m_indexCount = bufferIndex.y;
}

void Profiler::updateBuffer(const U32 ascii_code, const vx::float2 position_x_texSlice, vx::float2 &cursorPos, const vx::uint2 textureSize, const __m128 &invTextureSize, const __m128 &color, vx::uint2 &bufferIndex)
{
	const F32 scale = 0.25f;
	__m128 vScale = { scale, scale, 0, 0 };

	if (ascii_code == '\n')
	{
		cursorPos.y -= 53.0f * scale;
		cursorPos.x = position_x_texSlice.x;
	}
	else
	{
		auto pAtlasEntry = m_pFont->getAtlasEntry(ascii_code);
		vx::float4 texRect = { pAtlasEntry->x, textureSize.y - pAtlasEntry->y - pAtlasEntry->height, pAtlasEntry->width, pAtlasEntry->height };
		vx::float2 atlasOffset = { pAtlasEntry->offsetX, pAtlasEntry->offsetY };

		vx::float2 entrySize = { texRect.z, texRect.w };
		vx::float4 currentPosition = { 0, 0, -1.0f, 0 };
		currentPosition.x = fmaf((atlasOffset.x + entrySize.x / 2.0f), scale, cursorPos.x);
		currentPosition.y = fmaf((atlasOffset.y - entrySize.y / 2.0f), scale, cursorPos.y);
		__m128 vCurrentPosition = vx::loadFloat(&currentPosition);

		__m128 vSource = _mm_loadu_ps(texRect);
		vSource = _mm_mul_ps(vSource, invTextureSize);
		__m128 vSourceSize = _mm_shuffle_ps(vSource, vSource, _MM_SHUFFLE(3, 2, 3, 2));

		static const __m128 texOffsets[4] =
		{
			{ 0, 0, 0, 0 },
			{ 1, 0, 0, 0 },
			{ 1, 1, 0, 0 },
			{ 0, 1, 0, 0 }
		};

		static const __m128 posOffsets[4] =
		{
			{ -0.5f, -0.5f, 0, 0 },
			{ 0.5f, -0.5f, 0, 0 },
			{ 0.5f, 0.5f, 0, 0 },
			{ -0.5f, 0.5f, 0, 0 }
		};

		__m128 vEntrySize = vx::loadFloat(&entrySize);

		for (U32 k = 0; k < 4; ++k)
		{
			U32 index = bufferIndex.x + k;

			auto pos = _mm_mul_ps(posOffsets[k], vEntrySize);
			pos = _mm_fmadd_ps(pos, vScale, vCurrentPosition);
			_mm_storeu_ps(&m_pVertices[index].inputPosition.x, pos);

			__m128 uv = _mm_fmadd_ps(texOffsets[k], vSourceSize, vSource);
			_mm_storeu_ps(&m_pVertices[index].inputTexCoords.x, uv);
			m_pVertices[index].inputTexCoords.z = position_x_texSlice.y;

			_mm_storeu_ps(&m_pVertices[index].inputColor.x, color);
		}
		bufferIndex.x += 4;
		bufferIndex.y += 6;

		cursorPos.x += pAtlasEntry->advanceX * scale;
	}
}

void Profiler::render(vx::gl::StateManager &stateManager)
{
	stateManager.enable(vx::gl::Capabilities::Blend);
	stateManager.disable(vx::gl::Capabilities::Depth_Test);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	auto fsProgram = m_pPipeline->getFragmentShader();
	glProgramUniform1ui(fsProgram, 0, m_textureIndex);
	stateManager.bindPipeline(m_pPipeline->getId());
	stateManager.bindVertexArray(m_vao.getId());
	glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);

	stateManager.enable(vx::gl::Capabilities::Depth_Test);
	stateManager.disable(vx::gl::Capabilities::Blend);
}

void Profiler::pushGpuMarker(const char *name)
{
	GpuThreadInfo& ti = m_gpuThreadInfo;
	GpuMarker& marker = ti.markers[ti.currentWriteId];
	assert(marker.frame != m_currentFrame && "looping: too many markers, no free slots available");

	glQueryCounter(marker.id_query_start, GL_TIMESTAMP);
	// Fill in marker
	marker.start = 0;
	marker.end = 0;
	marker.layer = ti.nb_pushed_markers;
	strncpy_s(marker.name, name, s_maxCharacters);
	//marker.color = color;
	marker.frame = m_currentFrame;
	incrementCycle(&ti.currentWriteId, s_markersPerThread);
	++ti.nb_pushed_markers;
}

void Profiler::popGpuMarker()
{
	GpuThreadInfo& ti = m_gpuThreadInfo;
	// Get the most recent marker that has not been closed yet
	//auto index = ti.currentWriteId - 1;
	auto index = (ti.currentWriteId == 0) ? s_markersPerThread - 1 : ti.currentWriteId - 1;
	//VX_ASSERT(index != -1, "Invalid index !");
	while (ti.markers[index].end != 0) // skip closed markers
		decrementCycle(&index, s_markersPerThread);
	GpuMarker& marker = ti.markers[index];

	marker.end = 1;
	glQueryCounter(marker.id_query_end, GL_TIMESTAMP);
	--ti.nb_pushed_markers;
}

void Profiler::pushCpuMarker(const char *name)
{
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);

	CpuThreadInfo& ti = m_cpuThreadInfo;
	Marker& marker = ti.markers[ti.currentWriteId];
	assert(marker.frame != m_currentFrame && "looping: too many markers, no free slots available");

	// Fill in marker
	marker.start = time.QuadPart;
	marker.end = 0;
	marker.layer = ti.nb_pushed_markers;
	strncpy_s(marker.name, name, s_maxCharacters);
	//marker.color = color;
	marker.frame = m_currentFrame;
	incrementCycle(&ti.currentWriteId, s_markersPerThread);
	++ti.nb_pushed_markers;
}

void Profiler::popCpuMarker()
{
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);

	CpuThreadInfo& ti = m_cpuThreadInfo;
	// Get the most recent marker that has not been closed yet
	//auto index = ti.currentWriteId - 1;
	auto index = (ti.currentWriteId == 0) ? s_markersPerThread - 1 : ti.currentWriteId - 1;
	//VX_ASSERT(index != -1, "Invalid index !");
	while (ti.markers[index].end != 0) // skip closed markers
		decrementCycle(&index, s_markersPerThread);

	Marker& marker = ti.markers[index];
	marker.end = time.QuadPart;

	--ti.nb_pushed_markers;
}
#endif
#endif