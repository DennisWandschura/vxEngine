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
#include "CpuProfiler.h"
#include <vxLib/Container/sorted_vector.h>
#include <vxLib/StringID.h>
#include <vxLib/memory.h>
#include <Windows.h>
#include "Font.h"
#include <vxGL/Buffer.h>
#include <vxGL/VertexArray.h>
#include <vxGL/gl.h>
#include <vxGL/ProgramPipeline.h>
#include <vxGL/StateManager.h>
#include <vxEngineLib/mutex.h>

namespace CpuProfilerCpp
{
	const vx::float4 g_threadColors[] =
	{
		{1, 0, 0, 1},
		{0, 1, 0, 1},
		{0, 0, 1, 1},
		{1, 1, 0, 0}
	};

	const f32 g_threadPositionOffset = 300.0f;

	static const u32 s_maxMarkersPerThread = 128;

	static const u8 s_maxCharacters = 31u;

	static const u8 s_maxCpuStringSize = 64;
	static const u32 s_maxCpuCharacters = s_maxCpuStringSize * s_maxMarkersPerThread;
	static const u32 s_maxVertices = s_maxCpuCharacters * 4u;
	static const u32 s_maxIndices = s_maxCpuCharacters * 6u;

	struct Vertex
	{
		vx::float3 inputPosition;
		vx::float3 inputTexCoords;
		vx::float4 inputColor;
	};

	class Profiler
	{
		static LARGE_INTEGER s_frequency;

		struct Marker
		{
			s64 start;
			s64 end;
			char name[s_maxCharacters];
			u8 layer;
		};

		struct Entry
		{
			char name[s_maxCharacters];
			u8 layer;
			s64 time;
			s64 maxTime;
		};

		int m_pushedMarkers;
		int m_currentWriteId;
		u32 m_entryCount;
		u32 m_vertexCount;
		u32 m_indexCount;
		u32 m_tid;
		vx::sorted_vector<vx::StringID, u32> m_entriesByName;
		std::unique_ptr<Marker[]> m_markers;
		std::unique_ptr<Entry[]> m_entries;
		std::unique_ptr<Vertex[]> m_pVertices;
		const Font* m_pFont;
		vx::mutex m_mutex;
		vx::float2 m_position;

		inline void	incrementCycle(s32* pval, size_t array_size)
		{
			s32 val = *pval;
			++val;

			if (val >= (s32)(array_size))
				val = 0;

			*pval = val;
		}

		inline void	decrementCycle(u32* pval, size_t array_size)
		{
			s64 val = *pval;
			--val;

			if (val < 0)
				val = (s64)(array_size - 1);

			*pval = val;
		}

		void updateBuffer(const u32 ascii_code, const vx::float2 &position_x_texSlice, const vx::uint2a &textureSize, const __m128 &invTextureSize, const __m128 &color,
			vx::uint2 *bufferIndex, vx::float2 *cursorPos)
		{
			const f32 scale = 0.25f;
			const __m128 vScale = { scale, scale, 0, 0 };

			const __m128 tmp = { -1.0f, 0, 0, 0 };

			if (ascii_code == '\n')
			{
				cursorPos->y -= 53.0f * scale;
				cursorPos->x = position_x_texSlice.x;
			}
			else
			{
				__m128 vCursorPos = { cursorPos->x, cursorPos->y, 0.0f, 0.0f };

				auto pAtlasEntry = m_pFont->getAtlasEntry(ascii_code);
				vx::float4a texRect(pAtlasEntry->x, textureSize.y - pAtlasEntry->y - pAtlasEntry->height, pAtlasEntry->width, pAtlasEntry->height);

				__m128 vAtlasPos = { pAtlasEntry->offsetX, pAtlasEntry->offsetY, 0.0f, 0.0f };

				__m128 vEntrySize = { texRect.z, -texRect.w, 0.0f, 0.0f };

				__m128 vCurrentPosition = _mm_div_ps(vEntrySize, vx::g_VXTwo);
				vCurrentPosition = _mm_add_ps(vCurrentPosition, vAtlasPos);
				vCurrentPosition = vx::fma(vCurrentPosition, vScale, vCursorPos);
				vCurrentPosition = _mm_movelh_ps(vCurrentPosition, tmp);

				__m128 vSource = _mm_mul_ps(texRect, invTextureSize);
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

				vEntrySize = _mm_shuffle_ps(texRect, texRect, _MM_SHUFFLE(3, 2, 3, 2));

				for (u32 k = 0; k < 4; ++k)
				{
					u32 index = bufferIndex->x + k;

					auto pos = _mm_mul_ps(posOffsets[k], vEntrySize);
					pos = vx::fma(pos, vScale, vCurrentPosition);
					_mm_storeu_ps(&m_pVertices[index].inputPosition.x, pos);

					__m128 uv = vx::fma(texOffsets[k], vSourceSize, vSource);
					_mm_storeu_ps(&m_pVertices[index].inputTexCoords.x, uv);
					m_pVertices[index].inputTexCoords.z = position_x_texSlice.y;

					_mm_storeu_ps(&m_pVertices[index].inputColor.x, color);
				}
				bufferIndex->x += 4;
				bufferIndex->y += 6;

				cursorPos->x = fmaf(pAtlasEntry->advanceX, scale, cursorPos->x);
			}
		}

		void writeBuffer(s32 strSize, const char* buffer, const vx::float2 &position_x_texSlice, const vx::uint2a &textureSize, const __m128 &vInvTexSize, const __m128 &color,
			vx::uint2* bufferIndex, vx::float2* cursorPos)
		{
			for (s32 i = 0; i < strSize; ++i)
			{
				char ascii_code = buffer[i];

				updateBuffer(ascii_code, position_x_texSlice, textureSize, vInvTexSize, color, bufferIndex, cursorPos);
			}
		}

		void writeGpuMarkers(f32 textureSlice, const vx::uint2a &textureSize, const __m128 &vInvTexSize, vx::uint2* bufferIndex, vx::float2 *cursorPos)
		{
			__m128 vColor = vx::loadFloat4(g_threadColors[m_tid]);

			char buffer[s_maxCpuStringSize];
			vx::float2 position_x_texSlice(cursorPos->x, textureSlice);

			for (auto i = 0u; i < m_entryCount; ++i)
			{
				auto &entry = m_entries[i];

				//int strSize = sprintf_s(buffer, "%s %.6f ms\n",it.name, it.time * 1.0e-6);
				int strSize = sprintf_s(buffer, "%s", entry.name);

				auto layerOffset = entry.layer * 10.0f;
				//cursorPos->x = fmaf(it.layer, 20.0f, cursorPos->x);

				auto currentPos = *cursorPos;

				currentPos.x += layerOffset;
				writeBuffer(strSize, buffer, position_x_texSlice, textureSize, vInvTexSize, vColor, bufferIndex, &currentPos);

				strSize = sprintf_s(buffer, "%.5f ms", f64(entry.time) * 0.001);
				currentPos.x = cursorPos->x + 110;
				writeBuffer(strSize, buffer, position_x_texSlice, textureSize, vInvTexSize, vColor, bufferIndex, &currentPos);

				strSize = sprintf_s(buffer, "%.5f ms\n", f64(entry.maxTime) * 0.001);
				currentPos.x = cursorPos->x + 205;
				writeBuffer(strSize, buffer, position_x_texSlice, textureSize, vInvTexSize, vColor, bufferIndex, &currentPos);

				entry.maxTime = 0;

				*cursorPos = currentPos;
			}
		}

	public:
		Profiler()
			:m_pushedMarkers(0),
			m_currentWriteId(0),
			m_entryCount(0),
			m_vertexCount(0),
			m_indexCount(0),
			m_tid(0),
			m_entriesByName(),
			m_markers(),
			m_entries(),
			m_pVertices(),
			m_pFont(nullptr),
			m_position(0, 0)
		{
			if (s_frequency.QuadPart == 0)
				QueryPerformanceFrequency(&s_frequency);
		}

		~Profiler()
		{

		}

		void initialize(const Font* font)
		{
			m_entriesByName.reserve(s_maxMarkersPerThread);
			m_markers = vx::make_unique<Marker[]>(s_maxMarkersPerThread);
			m_pVertices = vx::make_unique<Vertex[]>(s_maxVertices);
			m_entries = vx::make_unique<Entry[]>(s_maxMarkersPerThread);
			m_pFont = font;
		}

		void frame()
		{
			m_pushedMarkers = 0;

			for (u32 i = 0; i < s_maxMarkersPerThread; ++i)
			{
				auto &marker = m_markers[i];

				if (marker.end != 0)
				{
					vx::StringID sid = vx::make_sid(marker.name);
					auto it = m_entriesByName.find(sid);

					if (it == m_entriesByName.end())
					{
						Entry entry;
						strncpy(entry.name, marker.name, s_maxCharacters);
						entry.layer = marker.layer;
						entry.maxTime = 0;

						m_entries[m_entryCount] = entry;

						it = m_entriesByName.insert(sid, m_entryCount);

						++m_entryCount;
					}

					auto &entry = m_entries[*it];
					entry.layer = marker.layer;
					entry.time = (marker.end - marker.start) * 1000000 / s_frequency.QuadPart;
					entry.maxTime = std::max(entry.maxTime, entry.time);
				}
			}
		}

		void update()
		{
				f32 textureSlice = m_pFont->getTextureEntry().getSlice();
				auto textureSize = m_pFont->getTextureEntry().getTextureSize();
				vx::float4a invTextureSize;
				invTextureSize.x = 1.0f / textureSize.x;
				invTextureSize.y = 1.0f / textureSize.y;

				//vx::float2(1.0f / textureSize.x, 1.0f / textureSize.y);
				auto vInvTexSize = _mm_shuffle_ps(invTextureSize.v, invTextureSize.v, _MM_SHUFFLE(1, 0, 1, 0));

				vx::uint2 bufferIndex = { 0, 0 };
				vx::float2 position = m_position;
				vx::lock_guard<vx::mutex> lock(m_mutex);
				writeGpuMarkers(textureSlice, textureSize, vInvTexSize, &bufferIndex, &position);

				//glNamedBufferSubData(m_vbo.getId(), 0, sizeof(Vertex) * bufferIndex.x, m_pVertices.get());

				m_vertexCount = bufferIndex.x;
				m_indexCount = bufferIndex.y;
		}

		void pushMarker(const char* id)
		{
			Marker &marker = m_markers[m_currentWriteId];

			LARGE_INTEGER current;
			QueryPerformanceCounter(&current);

			marker.start = current.QuadPart;
			marker.end = 0;
			marker.layer = m_pushedMarkers;
			strncpy(marker.name, id, s_maxCharacters);
			incrementCycle(&m_currentWriteId, s_maxMarkersPerThread);
			++m_pushedMarkers;
		}

		void popMarker()
		{
			u32 index = (m_currentWriteId == 0) ? s_maxMarkersPerThread - 1 : m_currentWriteId - 1;

			while (m_markers[index].end != 0) // skip closed markers
				decrementCycle(&index, (u32)s_maxMarkersPerThread);

			auto& marker = m_markers[index];

			LARGE_INTEGER current;
			QueryPerformanceCounter(&current);

			marker.end = current.QuadPart;

			--m_pushedMarkers;
		}

		void setPosition(const vx::float2 &p)
		{
			m_position = p;
		}

		const Vertex* getVertices() const
		{
			return m_pVertices.get();
		}

		u32 getVertexCount() const
		{
			return m_vertexCount;
		}

		u32 getIndexCount() const
		{
			return m_indexCount;
		}

		vx::mutex& getMutex()
		{
			return m_mutex;
		}

		void setTid(u32 tid)
		{
			m_tid = tid;
		}
	};

	class ProfileRenderer
	{
		static vx::float2 s_resolution;

		std::vector<Profiler*> m_profilers;
		const vx::gl::ProgramPipeline* m_pipeline;
		vx::gl::VertexArray m_vao;
		u32 m_indexCount;
		u32 m_textureIndex;
		vx::gl::Buffer m_vbo;
		vx::gl::Buffer m_ibo;
		vx::float2 m_position;
		vx::mutex m_mutex;

		void createVbo()
		{
			vx::gl::BufferDescription desc;
			desc.bufferType = vx::gl::BufferType::Array_Buffer;
			desc.flags = vx::gl::BufferStorageFlags::Write | vx::gl::BufferStorageFlags::Dynamic_Storage;
			desc.immutable = 1;
			desc.pData = nullptr;
			desc.size = sizeof(Vertex) * s_maxVertices;

			m_vbo.create(desc);
		}

		void createIbo()
		{
			std::unique_ptr<u32[]> pIndices = vx::make_unique<u32[]>(s_maxIndices);
			for (u32 i = 0, j = 0; i < s_maxIndices; i += 6, j += 4)
			{
				pIndices[i] = j;
				pIndices[i + 1] = j + 1;
				pIndices[i + 2] = j + 2;

				pIndices[i + 3] = j + 2;
				pIndices[i + 4] = j + 3;
				pIndices[i + 5] = j;
			}

			vx::gl::BufferDescription desc;
			desc.bufferType = vx::gl::BufferType::Element_Array_Buffer;
			desc.flags = 0;
			desc.immutable = 1;
			desc.pData = pIndices.get();
			desc.size = sizeof(u32) * s_maxIndices;

			m_ibo.create(desc);
		}

		void createVao()
		{
			m_vao.create();

			//vx::float3 inputPosition;
			//vx::float3 inputTexCoords;
			//vx::float4 inputColor;

			m_vao.enableArrayAttrib(0);
			m_vao.enableArrayAttrib(1);
			m_vao.enableArrayAttrib(2);

			m_vao.arrayAttribBinding(0, 0);
			m_vao.arrayAttribBinding(1, 0);
			m_vao.arrayAttribBinding(2, 0);

			m_vao.arrayAttribFormatF(0, 3, 0, 0);
			m_vao.arrayAttribFormatF(1, 3, 0, sizeof(vx::float3));
			m_vao.arrayAttribFormatF(2, 4, 0, sizeof(vx::float3) * 2);

			m_vao.bindIndexBuffer(m_ibo);
			m_vao.bindVertexBuffer(m_vbo, 0, 0, sizeof(Vertex));
		}

	public:
		ProfileRenderer() :m_indexCount(0), m_textureIndex(0){}
		~ProfileRenderer(){}

		void initialize(const vx::gl::ProgramPipeline* pPipeline, u32 textureIndex, const vx::float2 &resolution)
		{
			createVbo();
			createIbo();
			createVao();

			m_pipeline = pPipeline;
			m_textureIndex = textureIndex;

			s_resolution = resolution;
		}

		void registerProfiler(Profiler* p)
		{
			m_profilers.push_back(p);
		}

		void update()
		{
			u32 vertexOffset = 0;
			m_indexCount = 0;
			for (auto &it : m_profilers)
			{
				auto &mutex = it->getMutex();

				vx::lock_guard<vx::mutex> guard(mutex);
				auto vertices = it->getVertices();
				auto vertexCount = it->getVertexCount();
				auto indexCount = it->getIndexCount();

				glNamedBufferSubData(m_vbo.getId(), sizeof(Vertex) * vertexOffset, sizeof(Vertex) * vertexCount, vertices);

				vertexOffset += vertexCount;
				m_indexCount += indexCount;
			}
		}

		void render()
		{
			auto fsProgram = m_pipeline->getFragmentShader();
			glProgramUniform1ui(fsProgram, 0, m_textureIndex);
			vx::gl::StateManager::bindPipeline(m_pipeline->getId());
			vx::gl::StateManager::bindVertexArray(m_vao);
			glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
		}

		void setPosition(const vx::float2 &position)
		{
			m_position = position;

			updatePosition();
		}

		void updatePosition()
		{
			auto pos = m_position;
			for (u32 i = 0; i < m_profilers.size(); ++i)
			{
				auto &it = m_profilers[i];

				it->setPosition(pos);

				pos.x += g_threadPositionOffset;
			}
		}

		const vx::float2& getPosition() const
		{
			return m_position;
		}

		u32 getProfilerCount() const
		{
			return m_profilers.size();
		}

		vx::mutex& getMutex()
		{
			return m_mutex;
		}
	};

	thread_local Profiler* s_profiler{ nullptr };
	LARGE_INTEGER Profiler::s_frequency{0};
	std::atomic<ProfileRenderer*> s_renderer{nullptr};
	vx::float2 ProfileRenderer::s_resolution{1920, 1080};
}

void CpuProfiler::initializeRenderer(const vx::gl::ProgramPipeline* pPipeline, u32 textureIndex, const vx::float2 &resolution)
{
	if (CpuProfilerCpp::s_renderer.load() == nullptr)
	{
		auto renderer = new CpuProfilerCpp::ProfileRenderer();
		renderer->initialize(pPipeline, textureIndex, resolution);

		CpuProfilerCpp::s_renderer.store(renderer);
	}
}

void CpuProfiler::initialize(const Font* font)
{
	if (CpuProfilerCpp::s_profiler == nullptr)
	{
		CpuProfilerCpp::s_profiler = new CpuProfilerCpp::Profiler();
		CpuProfilerCpp::s_profiler->initialize(font);

		puts("creating cpu profiler");
		while (CpuProfilerCpp::s_renderer.load() == nullptr)
			;

		auto &mutex = CpuProfilerCpp::s_renderer.load()->getMutex();
		vx::lock_guard<vx::mutex> guard(mutex);

		CpuProfilerCpp::s_renderer.load()->registerProfiler(CpuProfilerCpp::s_profiler);
		CpuProfilerCpp::s_renderer.load()->updatePosition();
		auto count = CpuProfilerCpp::s_renderer.load()->getProfilerCount();

		CpuProfilerCpp::s_profiler->setTid(count);

		//p.x += count * CpuProfilerCpp::g_threadPositionOffset;
	//	CpuProfilerCpp::s_profiler->setPosition(p);

		puts("created cpu profiler");
	}
}

void CpuProfiler::shutdown()
{
	if (CpuProfilerCpp::s_profiler != nullptr)
	{
		delete CpuProfilerCpp::s_profiler;
		CpuProfilerCpp::s_profiler = nullptr;
	}
}

void CpuProfiler::shutdownRenderer()
{
	if (CpuProfilerCpp::s_renderer.load() != nullptr)
	{
		delete CpuProfilerCpp::s_renderer.load();
		CpuProfilerCpp::s_renderer.store(nullptr);
	}
}

void CpuProfiler::updateRenderer()
{
	CpuProfilerCpp::s_renderer.load()->update();
}

void CpuProfiler::render()
{
	CpuProfilerCpp::s_renderer.load()->render();
}

void CpuProfiler::setPosition(const vx::float2 &position)
{
	auto &mutex = CpuProfilerCpp::s_renderer.load()->getMutex();
	vx::lock_guard<vx::mutex> guard(mutex);
	CpuProfilerCpp::s_renderer.load()->setPosition(position);
}

void CpuProfiler::frame()
{
	CpuProfilerCpp::s_profiler->frame();
}

void CpuProfiler::update()
{
	CpuProfilerCpp::s_profiler->update();
}

void CpuProfiler::pushMarker(const char* id)
{
	CpuProfilerCpp::s_profiler->pushMarker(id);
}

void CpuProfiler::popMarker()
{
	CpuProfilerCpp::s_profiler->popMarker();
}