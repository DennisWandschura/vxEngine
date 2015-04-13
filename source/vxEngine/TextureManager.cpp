#include "TextureManager.h"
#include "TextureFile.h"

TextureRef::TextureRef()
	:m_textureId(0), m_slice(0)
{
}

TextureRef::TextureRef(U32 textureId, U32 slice, vx::uint2 textureSize, U8 isArray)
	: m_textureId(textureId), m_slice(slice), m_textureSize(textureSize)
{
	if (isArray != 0)
	{
		U32 oldSlize = m_slice;
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

U32 TextureRef::getTextureId() const noexcept
{
	return m_textureId;
}

U32 TextureRef::getSlice() const noexcept
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

/*

*/

TextureManager::TextureWrapper::TextureWrapper()
:m_texture(),
m_firstFreeSlice(0),
m_freeSlices(0),
m_pSlices()
{
}
TextureManager::TextureWrapper::TextureWrapper(TextureWrapper &&rhs)
	: m_texture(std::move(rhs.m_texture)),
	m_firstFreeSlice(rhs.m_firstFreeSlice),
	m_freeSlices(rhs.m_freeSlices),
	m_pSlices(std::move(rhs.m_pSlices))
{
}

TextureManager::TextureWrapper& TextureManager::TextureWrapper::operator = (TextureWrapper &&rhs)
{
	if (this != &rhs)
	{
		m_texture = std::move(rhs.m_texture);
		m_firstFreeSlice = rhs.m_firstFreeSlice;
		m_freeSlices = rhs.m_freeSlices;
		m_pSlices = std::move(rhs.m_pSlices);
	}

	return *this;
}

TextureManager::TextureBucket::TextureBucket()
	:m_pTextures(),
	m_size(0),
	m_capacity(0)
{

}

TextureManager::TextureBucket::TextureBucket(TextureBucket &&rhs)
	:m_pTextures(std::move(rhs.m_pTextures)),
	m_size(rhs.m_size),
	m_capacity(rhs.m_capacity)
{

}

TextureManager::TextureBucket& TextureManager::TextureBucket::operator = (TextureBucket &&rhs)
{
	if (this != &rhs)
	{
		m_pTextures = std::move(rhs.m_pTextures);
		m_size = rhs.m_size;
		m_capacity = rhs.m_capacity;
	}
	return *this;
}

TextureManager::TextureManager()
	:m_textureBuckets(),
	m_wrappers()
{
}

void TextureManager::reserveBuckets(U32 n)
{
	m_textureBuckets.reserve(n);
}

TextureManager::TextureBucket* TextureManager::findBucket(const vx::ushort3 textureSize, U8 miplevels, vx::gl::TextureType type, vx::gl::TextureFormat format)
{
	TextureCmp cmp;
	cmp.m_size = textureSize;
	cmp.miplevels = miplevels;
	cmp.format = format;
	cmp.type = type;

	TextureBucket *pBucket = nullptr;

	auto it = m_textureBuckets.find(cmp);
	if (it != m_textureBuckets.end())
	{
		pBucket = &*it;
	}

	return pBucket;
}

TextureRef TextureManager::createTexture2DSlice(TextureBucket *pBucket, vx::uint2 size, vx::gl::DataType dataType, const void *pData)
{
	TextureRef ref;
	for (auto i = 0u; i < pBucket->m_size; ++i)
	{
		auto &texture = pBucket->m_pTextures[i];
		assert(texture.m_pSlices);

		// we have space left
		if (texture.m_freeSlices != 0)
		{
			auto sliceIndex = texture.m_firstFreeSlice;
			texture.m_firstFreeSlice = texture.m_pSlices[sliceIndex];

			vx::gl::TextureCommitDescription commitDesc;
			commitDesc.miplevel = 0;
			commitDesc.offset = vx::uint3(0, 0, sliceIndex);
			commitDesc.size = vx::uint3(size.x, size.y, 1);
			commitDesc.commit = 1;

			// commit memory
			texture.m_texture.commit(commitDesc);
			// load data
			if (pData)
			{
				vx::gl::TextureSubImageDescription subImageDesc;
				subImageDesc.miplevel = 0;
				subImageDesc.offset = vx::uint3(0, 0, sliceIndex);
				subImageDesc.size = vx::uint3(size.x, size.y, 1);
				subImageDesc.dataType = dataType;
				subImageDesc.p = pData;

				texture.m_texture.subImage(subImageDesc);
			}
			// if miplevels > 1

			vx::uint2 textureSize = { texture.m_texture.getSize().x, texture.m_texture.getSize().y };
			ref = TextureRef(texture.m_texture.getId(), sliceIndex, textureSize, 1);

			--texture.m_freeSlices;
		}
	}
	return ref;
}

void TextureManager::createBucket(U32 bucketSize, const vx::ushort3 textureSize, U8 miplevels, vx::gl::TextureType type, vx::gl::TextureFormat format)
{
	TextureCmp cmp;
	cmp.m_size = textureSize;
	cmp.miplevels = miplevels;
	cmp.format = format;
	cmp.type = type;

	// try to find bucket
	auto it = m_textureBuckets.find(cmp);
	if (it == m_textureBuckets.end())
	{
		// no bucket exists yet, so create it
		TextureBucket bucket;
		bucket.m_pTextures = std::make_unique<TextureWrapper[]>(bucketSize);
		//	bucket.m_pUsedFlag = std::make_unique<U8[]>(bucketSize);
		bucket.m_size = 0;
		bucket.m_capacity = bucketSize;

		m_textureBuckets.insert(cmp, std::move(bucket));
	}
	// else do nothing
}

U64 TextureManager::createTexture(const vx::ushort3 size, U8 miplevels, vx::gl::TextureType type, vx::gl::TextureFormat format)
{
	// try to find bucket
	auto pBucket = findBucket(size, miplevels, type, format);
	if (!pBucket)
		return 0;

	// make sure the bucket has space left
	if (pBucket->m_size == pBucket->m_capacity)
	{
		return 0;
	}

	// create texture
	vx::gl::TextureDescription desc;
	desc.size.x = size.x;
	desc.size.y = size.y;
	desc.size.z = size.z;
	desc.miplevels = miplevels;
	desc.type = type;
	desc.format = format;
	desc.sparse = 1;

	TextureWrapper newWrapper;
	newWrapper.m_texture.create(desc);
	newWrapper.m_texture.makeTextureResident();
	// if texture array, create slice array
	if (type == vx::gl::TextureType::Texture_2D_Array)
	{
		newWrapper.m_firstFreeSlice = 0;
		newWrapper.m_freeSlices = size.z;
		newWrapper.m_pSlices = std::make_unique<U16[]>(size.z);

		for (auto i = 0u; i < size.z; ++i)
		{
			newWrapper.m_pSlices[i] = i + 1;
		}
	}
	U64 handle = newWrapper.m_texture.getTextureHandle();

	U32 index = pBucket->m_size++;
	auto &wrapperRef = pBucket->m_pTextures[index];
	wrapperRef = std::move(newWrapper);

	m_wrappers.insert(wrapperRef.m_texture.getId(), &wrapperRef);
	return handle;
}

TextureRef TextureManager::load(const TextureFile &f, U8 mipLevels, U8 srgb)
{
	auto textureSize = f.getSize();
	U32 channels = f.getChannels();

	vx::gl::TextureFormat format = vx::gl::TextureFormat::SRGBA8;
	if (srgb == 0)
	{
		if (channels == 3)
			format = vx::gl::TextureFormat::RGB8;
		else
			format = vx::gl::TextureFormat::RGBA8;
	}
	else
	{
		if (channels == 3)
			format = vx::gl::TextureFormat::SRGB8;
	}

	TextureRef result;
	vx::ushort3 sz = { textureSize.x, textureSize.y, 1 };
	auto pBucket = findBucket(sz, mipLevels, vx::gl::TextureType::Texture_2D_Array, format);
	if (pBucket)
	{
		result = createTexture2DSlice(pBucket, textureSize, vx::gl::DataType::Unsigned_Byte, f.get(0));
	}

	return result;
}

TextureRef TextureManager::load(const vx::ushort3 size, U8 miplevels, vx::gl::TextureType type, vx::gl::TextureFormat format, vx::gl::DataType dataType, const void *ptr)
{
	TextureRef ref;
	auto pBucket = findBucket(size, miplevels, type, format);
	if (pBucket)
	{
		assert(type != vx::gl::TextureType::Texture_1D_Array);
		assert(type == vx::gl::TextureType::Texture_2D_Array ||
			type == vx::gl::TextureType::Texture_2D_MS_Array);

		vx::uint2 sz = { size.x, size.y };
		ref = createTexture2DSlice(pBucket, sz, dataType, ptr);
	}

	return ref;
}

U64 TextureManager::getTextureHandle(const TextureRef &ref)
{
	U64 handle = 0;

	auto it = m_wrappers.find(ref.getTextureId());
	if (it != m_wrappers.end())
	{
		handle = (*it)->m_texture.getTextureHandle();
	}

	return handle;
}

void TextureManager::release(TextureRef &&ref)
{
	auto texId = ref.getTextureId();

	auto it = m_wrappers.find(texId);
	if (it == m_wrappers.end())
		return;

	auto pWrapper = (*it);
	if (ref.isArray())
	{
		auto sliceIndex = ref.getSlice();
		auto &slice = pWrapper->m_pSlices[sliceIndex];
		// make sure slice is really used
		assert(slice == s_invalid);
		// free memory of slice
		auto texSize = pWrapper->m_texture.getSize();

		vx::gl::TextureCommitDescription desc;
		desc.miplevel = 0;
		desc.offset = vx::uint3(0, 0, sliceIndex);
		desc.size = vx::uint3(texSize.x, texSize.y, 1);
		desc.commit = 0;

		pWrapper->m_texture.commit(desc);
		// put slice back in pool
		slice = pWrapper->m_pSlices[pWrapper->m_firstFreeSlice];
		pWrapper->m_firstFreeSlice = sliceIndex;

		++pWrapper->m_freeSlices;
	}
	else
	{
		// not handled yet
		assert("false");
	}
	//
	ref.makeInvalid();
}