#pragma once

#include "FontAtlas.h"
#include "TextureManager.h"

class Font
{
	TextureRef m_texture;
	FontAtlas m_atlas;						// 32

public:
	Font();
	Font(TextureRef &&textureRef, FontAtlas &&fontAtlas);
	Font(Font &&other);
	Font(const Font&) = delete;
	~Font();

	Font& operator=(const Font&) = delete;
	Font& operator=(Font &&rhs);

	const FontAtlasEntry* getAtlasEntry(U32 code) const;
	const TextureRef& getTextureEntry() const
	{
		return m_texture;
	}
};