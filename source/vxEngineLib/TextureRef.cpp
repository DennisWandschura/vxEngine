#include <vxEngineLib/TextureRef.h>

TextureRef::TextureRef()
	:m_textureId(0), m_slice(0)
{
}

TextureRef::TextureRef(u32 textureId, u32 slice, const vx::uint2 &textureSize, u8 isArray)
	: m_textureId(textureId), m_slice(slice), m_textureSize(textureSize)
{
	if (isArray != 0)
	{
		u32 oldSlize = m_slice;
		m_slice |= (1 << 31);
		VX_ASSERT(oldSlize == getSlice());
	}
}

TextureRef::TextureRef(TextureRef &&rhs)
	:m_textureId(rhs.m_textureId),
	m_slice(rhs.m_slice),
	m_textureSize(rhs.m_textureSize)
{
	rhs.m_textureId = 0;
}

TextureRef& TextureRef::operator=(TextureRef &&rhs)
{
	if (this != &rhs)
	{
		m_textureId = rhs.m_textureId;
		m_slice = rhs.m_slice;
		m_textureSize = rhs.m_textureSize;

		rhs.m_textureId = 0;
	}
	return *this;
}

void TextureRef::makeInvalid()
{
	m_textureId = 0;
}

u32 TextureRef::getTextureId() const noexcept
{
	return m_textureId;
}

u32 TextureRef::getSlice() const noexcept
{
	return (m_slice & 0x7FFFFFFF);
}

bool TextureRef::isArray() const noexcept
{
	return ((m_slice >> 31) != 0);
}

bool TextureRef::isValid() const noexcept
{
	return m_textureId != 0;
}