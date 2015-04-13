#pragma once

class TextureFile;

#include <vxLib\gl\Texture.h>
#include <memory>
#include <vxLib\Container\sorted_vector.h>

class TextureRef
{
	U32 m_textureId;
	U32 m_slice;
	vx::uint2a m_textureSize;

public:
	TextureRef();
	TextureRef(U32 textureId, U32 slice, vx::uint2 textureSize, U8 isArray);

	TextureRef(const TextureRef&) = delete;
	TextureRef(TextureRef &&rhs);

	TextureRef& operator=(const TextureRef&) = delete;
	TextureRef& operator=(TextureRef &&rhs);

	void makeInvalid();

	U32 getTextureId() const noexcept;

	U32 getSlice() const noexcept;
	const vx::uint2a& getTextureSize() const noexcept{ return m_textureSize; }

	bool isArray() const noexcept;
	bool isValid() const noexcept;
};

class TextureManager
{
	static const U16 s_invalid{ -1 };
	struct TextureCmp
	{
		vx::ushort3 m_size;
		U8 miplevels;
		vx::gl::TextureType type;
		vx::gl::TextureFormat format;

		friend bool operator<(const TextureCmp &lhs, const TextureCmp &rhs)
		{
			if (lhs.type < rhs.type)
				return true;
			else if (lhs.type == rhs.type)
			{
				if (lhs.type == vx::gl::TextureType::Texture_2D_Array)
				{
					return (lhs.format < rhs.format) ||
						(lhs.format == rhs.format && lhs.m_size.x < rhs.m_size.x) ||
						(lhs.format == rhs.format && lhs.m_size.x == rhs.m_size.x && lhs.m_size.y < rhs.m_size.y) ||
						(lhs.format == rhs.format && lhs.m_size.x == rhs.m_size.x && lhs.m_size.y == rhs.m_size.y && lhs.miplevels < rhs.miplevels);
				}
				else
				{
					return (lhs.format < rhs.format) ||
						(lhs.format == rhs.format && lhs.m_size.x < rhs.m_size.x) ||
						(lhs.format == rhs.format && lhs.m_size.x == rhs.m_size.x && lhs.m_size.y < rhs.m_size.y) ||
						(lhs.format == rhs.format && lhs.m_size.x == rhs.m_size.x && lhs.m_size.y == rhs.m_size.y && lhs.m_size.z < rhs.m_size.z) ||
						(lhs.format == rhs.format && lhs.m_size.x == rhs.m_size.x && lhs.m_size.y == rhs.m_size.y && lhs.m_size.z == rhs.m_size.z && lhs.miplevels < rhs.miplevels);
				}
			}
			else
			{
				return false;
			}
		}
	};

	struct TextureWrapper
	{
		vx::gl::Texture m_texture;
		U16 m_firstFreeSlice;
		U16 m_freeSlices;
		std::unique_ptr<U16[]> m_pSlices;

		TextureWrapper();
		TextureWrapper(TextureWrapper &&rhs);

		TextureWrapper& operator=(TextureWrapper &&rhs);
	};

	struct TextureBucket
	{
		std::unique_ptr<TextureWrapper[]> m_pTextures;
		U32 m_size;
		U32 m_capacity;

		TextureBucket();
		TextureBucket(TextureBucket &&rhs);

		TextureBucket& operator=(TextureBucket &&rhs);
	};

	vx::sorted_vector<TextureCmp, TextureBucket> m_textureBuckets;
	vx::sorted_vector<U32, TextureWrapper*> m_wrappers;

	TextureBucket* findBucket(const vx::ushort3 textureSize, U8 miplevels, vx::gl::TextureType type, vx::gl::TextureFormat format);
	TextureRef createTexture2DSlice(TextureBucket *pBucket, const vx::uint2 size, vx::gl::DataType dataType, const void *pData);

public:
	TextureManager();
	void reserveBuckets(U32 n);

	void createBucket(U32 bucketSize, const vx::ushort3 textureSize, U8 miplevels, vx::gl::TextureType type, vx::gl::TextureFormat format);

	U64 createTexture(const vx::ushort3 size, U8 miplevels, vx::gl::TextureType type, vx::gl::TextureFormat format);

	// texture files are always loaded into 2d arrays
	TextureRef load(const TextureFile &f, U8 mipLevels, U8 srgb);

	TextureRef load(const vx::ushort3 size, U8 miplevels, vx::gl::TextureType type, vx::gl::TextureFormat format, vx::gl::DataType dataType = (vx::gl::DataType)0, const void *ptr = nullptr);

	U64 getTextureHandle(const TextureRef &ref);

	void release(TextureRef &&ref);
};