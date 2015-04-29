#pragma once

#include <vxLib/gl/Base.h>

namespace Graphics
{
	enum class RenderMode : U8{ DrawArrays, DrawElements, MultiDrawElementsIndirect };
	enum class PrimitiveMode : U8{ Points, Triangles };

	struct DrawCommandDescription
	{
		U32 indirectBuffer{ 0 };
		U32 count{0};
		PrimitiveMode primitiveMode{ PrimitiveMode::Triangles };
		vx::gl::DataType dataType{ vx::gl::DataType::Unsigned_Byte};
		RenderMode renderMode{ RenderMode::DrawArrays };

		bool isValid() const;
	};

	class DrawCommand
	{
		U32 m_indirectBuffer{ 0 };
		U32 m_count{ 0 };
		U16 m_dataType{ 0 };
		U8 m_primitiveMode{ 0 };
		RenderMode m_renderMode{ RenderMode::DrawArrays };

		void drawArrays() const;
		void drawElements() const;
		void multiDrawElementsIndirect() const;

	public:
		DrawCommand() = default;

		void initialize(const DrawCommandDescription &desc);

		void draw() const;

		void setDrawCount(U32 count) { m_count = count; }
		void setPrimitiveMode(PrimitiveMode mode);
		void setRenderMode(RenderMode renderMode);
		void setDataType(vx::gl::DataType dataType);
	};
}