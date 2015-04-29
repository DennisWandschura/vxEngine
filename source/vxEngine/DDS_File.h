#pragma once
#include <vxLib/types.h>
#include <vector>

struct DXTColBlock;

struct DXT3AlphaBlock;

struct DXT5AlphaBlock;

class Surface
{
	U32 m_width;
	U32 m_height;
	U32 m_depth;
	U32 m_size;
	U8* m_pixels;

public:
	Surface();
	Surface(const Surface&) = delete;
	Surface(Surface &&rhs);
	~Surface();

	Surface& operator=(const Surface&) = delete;
	Surface& operator=(Surface &&rhs);

	void create(U32 width, U32 height, U32 depth, U32 size, U8* pixels);

	void clear();

	U32 getWidth() const { return m_width; }
	U32 getHeight() const { return m_height; }
	U32 getDepth() const { return m_depth; }
	U32 getSize() const { return m_size; }

	U8* getPixels() { return m_pixels; }
	const U8* getPixels() const { return m_pixels; }
};

class Texture : public Surface
{
	std::vector<Surface> m_mipmaps;

public:
	Texture();
	Texture(const Texture&) = delete;
	Texture(Texture &&rhs);

	Texture& operator=(const Texture&) = delete;
	Texture& operator=(Texture &&rhs);

	void create(U32 width, U32 height, U32 depth, U32 size, U8* pixels);

	inline void addMipmap(Surface &&mipmap)
	{
		m_mipmaps.push_back(std::move(mipmap));
	}

	void clear();

	U32 getMipmapCount() const { return m_mipmaps.size(); }
	Surface* getMipmap(U32 i) { return &m_mipmaps[i]; }
	const Surface* getMipmap(U32 i) const { return &m_mipmaps[i]; }
};

class DDS_File
{
	enum class Type{ Flat, Cubemap, Volume };

	std::vector<Texture> m_images;
	U32 m_components;
	Type m_type;
	U32 m_format;

	unsigned int clamp_size(unsigned int size);

	unsigned int size_dxtc(unsigned int width, unsigned int height);

	unsigned int size_rgb(unsigned int width, unsigned int height);

	inline void swap_endian(void *val);

	void flip(Surface* surface);

	void flip_texture(Texture* texture);

	void swap(void *byte1, void *byte2, unsigned int size);

	void flip_blocks_dxtc1(DXTColBlock *line, unsigned int numBlocks);

	void flip_blocks_dxtc3(DXTColBlock *line, unsigned int numBlocks);

	void flip_dxt5_alpha(DXT5AlphaBlock *block);

	void flip_blocks_dxtc5(DXTColBlock *line, unsigned int numBlocks);

public:
	bool loadFromFile(const char* file, bool flipImage = true);

	bool isCompressed() const;

	const Texture& getTexture(U32 i) const { return m_images[i]; }
};
