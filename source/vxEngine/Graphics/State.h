#pragma once

#include <vxLib/types.h>
#include <string>

namespace Graphics
{
	class State
	{
		U32 m_fbo;
		U32 m_vao;
		U32 m_pipeline;
		U32 m_indirectBuffer;
		U32 m_paramBuffer;

		U8 m_blendState;
		U8 m_depthTestState;

	public:
		State();
		~State();

		void set(U32 fbo, U32 vao, U32 pipeline, U32 indirectBuffer, U32 paramBuffer = 0);

		void setDepthTest(bool b);
		void setBlendState(bool b);

		void update();

		void compile(std::string* str);
	};
}