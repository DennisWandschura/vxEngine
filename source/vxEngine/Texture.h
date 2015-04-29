#pragma once

#include "TextureLayer.h"

namespace Graphics
{
	class Texture : public TextureLayer
	{
		std::unique_ptr<TextureLayer[]> m_mipmaps;
		U32 m_mipmapCount;

	public:
		Texture();

		void create(const vx::ushort3 &dim, U32 size, std::unique_ptr<U8> &&data, U32 mipmapCount);

		TextureLayer& getMipmap(U32 i);
		const TextureLayer& getMipmap(U32 i) const;
	};
}