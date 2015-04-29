#include "RenderCommand.h"
#include <vxLib/gl/ProgramPipeline.h>
#include <vxLib/gl/VertexArray.h>

void RenderCommand::initialize(const vx::gl::VertexArray &vao, const vx::gl::ProgramPipeline &pipeline, const vx::uint2 &resolution)
{
	m_vao = vao.getId();
	m_pipeline = pipeline.getId();
	m_resolution = resolution;
}