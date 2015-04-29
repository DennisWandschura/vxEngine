#pragma once

#include <vxLib/math/Vector.h>
#include <memory>

namespace Graphics
{
	class TextureLayer
	{
		std::unique_ptr<U8> m_data;
		vx::ushort3 m_dimension;
		U32 m_size;

	public:
		TextureLayer();
		TextureLayer(const TextureLayer&) = delete;
		TextureLayer(TextureLayer &&rhs);

		void create(const vx::ushort3 &dim, U32 size, std::unique_ptr<U8> &&data);
		void clear();

		virtual ~TextureLayer();

		const vx::ushort3& getDim() const;
		U32 getSize() const;
		const U8* getData() const;
	};
}