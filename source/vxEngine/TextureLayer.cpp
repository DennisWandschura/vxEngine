#include "TextureLayer.h"

namespace Graphics
{
	TextureLayer::TextureLayer()
		:m_data(),
		m_dimension(0, 0, 0),
		m_size(0)
	{

	}

	TextureLayer::TextureLayer(TextureLayer &&rhs)
		:m_data(std::move(rhs.m_data)),
		m_dimension(rhs.m_dimension),
		m_size(rhs.m_size)
	{

	}

	TextureLayer::~TextureLayer()
	{

	}

	void TextureLayer::create(const vx::ushort3 &dim, U32 size, std::unique_ptr<U8> &&data)
	{
		m_data = std::move(data);
		m_dimension = dim;
		m_size = size;
	}

	void TextureLayer::clear()
	{
		m_data.reset();
		m_dimension = vx::ushort3(0, 0,0);
		m_size = 0;
	}

	const U8* TextureLayer::getData() const
	{
		return m_data.get();
	}

	const vx::ushort3& TextureLayer::getDim() const
	{
		return m_dimension;
	}

	U32 TextureLayer::getSize() const
	{
		return m_size;
	}
}