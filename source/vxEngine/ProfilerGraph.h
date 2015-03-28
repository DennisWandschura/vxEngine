#pragma once

namespace vx
{
	namespace gl
	{
		class ProgramPipeline;
		class StateManager;
		class ShaderManager;
	}
}

#include <vxLib/math/Vector.h>
#include <vxLib/gl/Buffer.h>
#include <vxLib/gl/VertexArray.h>

class ProfilerGraph
{
	static const U16 s_sampleCount = 500u;
	static I64 s_frequency;
	static const U8 s_queryCount = 3;

	I64 m_currentStart{0};
	U32 m_queryGpuStart[s_queryCount];
	U32 m_queryGpuEnd[s_queryCount];
	U32 m_currentQuery{0};
	F32 m_scale{ 1.0f };
	vx::float2 m_position{ 0, 0 };
	vx::float2 m_entriesCpu[s_sampleCount];
	vx::float2 m_entriesGpu[s_sampleCount];
	const vx::gl::ProgramPipeline *m_pPipeline{ nullptr };
	vx::gl::VertexArray m_vao;
	U32 m_indexCount{0};
	vx::gl::Buffer m_vbo;
	vx::gl::Buffer m_ibo;
	vx::gl::Buffer m_vboColor;

public:
	ProfilerGraph() = default;
	bool initialize(const vx::gl::ShaderManager &shaderManager, F32 targetMs);

	void startCpu();
	void endCpu();

	void startGpu();
	void endGpu();

	// call at begin of frame
	void frame(F32 frameTime);
	// updates buffers
	void update();
	void render();
};