#include "Texture.h"

namespace Graphics
{
	Texture::Texture()
		:TextureLayer(),
		m_mipmaps(),
		m_mipmapCount()
	{

	}

	void Texture::create(const vx::ushort3 &dim, U32 size, std::unique_ptr<U8> &&data, U32 mipmapCount)
	{
		TextureLayer::create(dim, size, std::move(data));

		if (mipmapCount != 0)
		{
			m_mipmaps = std::make_unique<TextureLayer[]>(mipmapCount);
			m_mipmapCount = mipmapCount;
		}
	}

	TextureLayer& Texture::getMipmap(U32 i)
	{
		return m_mipmaps[i];
	}

	const TextureLayer& Texture::getMipmap(U32 i) const
	{
		return m_mipmaps[i];
	}
}