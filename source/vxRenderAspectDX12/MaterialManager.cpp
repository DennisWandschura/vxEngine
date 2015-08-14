#include "MaterialManager.h"
#include <d3d12.h>
#include <vxEngineLib/Material.h>
#include <vxEngineLib/ResourceAspectInterface.h>
#include <vxEngineLib/Graphics/Texture.h>
#include "UploadManager.h"

struct MaterialManager::MaterialEntry
{
	u32 slice;
};

MaterialManager::MaterialManager()
	:m_entries(),
	m_freelistData(),
	m_freelist(),
	m_textureBuffer(),
	m_textureHeap(),
	m_textureOffset(0)
{

}

MaterialManager::~MaterialManager()
{

}

bool MaterialManager::createHeap(d3d::Device* device)
{
	D3D12_HEAP_PROPERTIES props
	{
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		0,
		0
	};

	D3D12_HEAP_DESC textureHeapDesc;
	textureHeapDesc.Alignment = 64u KBYTE;
	textureHeapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
	textureHeapDesc.Properties = props;
	textureHeapDesc.SizeInBytes = d3d::getAlignedSize(512u MBYTE, 64u KBYTE);
	if (!m_textureHeap.create(textureHeapDesc, device))
		return false;

	return true;
}

bool MaterialManager::createSrgbTextureArray(const vx::uint2 &textureResolution, u32 maxTextureCount, d3d::Device* device)
{
	D3D12_RESOURCE_DESC resDesc;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Alignment = 64 KBYTE;
	resDesc.Width = textureResolution.x;
	resDesc.Height = textureResolution.y;
	resDesc.DepthOrArraySize = maxTextureCount;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	if (!m_textureHeap.createResource(resDesc, 0, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, m_textureBuffer.getAddressOf(), device))
		return false;

	return true;
}

bool MaterialManager::initialize(const vx::uint2 &textureResolution, u32 maxTextureCount, d3d::Device* device)
{
	if (!createHeap(device))
		return false;

	if (!createSrgbTextureArray(textureResolution, maxTextureCount, device))
		return false;

	m_textureOffset = textureResolution.x * textureResolution.y * 4;

	m_freelistData = std::make_unique<u32[]>(maxTextureCount);
	m_freelist.create((u8*)m_freelistData.get(), maxTextureCount, sizeof(u32));

	return true;
}

void MaterialManager::addTexture(u32 slice, const vx::StringID &sid, const ResourceAspectInterface* resourceAspect, UploadManager* uploadManager)
{
	auto texture = resourceAspect->getTexture(sid);

	auto format = texture->getFormat();
	auto &face = texture->getFace(0);

	auto dim = face.getDimension();
	auto dataSize = face.getSize();
	auto data = face.getPixels();

	UploadTaskTextureDesc desc;
	desc.dst = m_textureBuffer.get();
	desc.dataSize = dataSize;
	desc.data = data;
	desc.dim.x = dim.x;
	desc.dim.y = dim.y;
	desc.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.rowPitch = dim.x * 4;
	desc.slice = slice;
	desc.state = D3D12_RESOURCE_STATE_GENERIC_READ;
	uploadManager->pushUploadTexture(desc);
}

bool MaterialManager::addMaterial(const Material* material, const ResourceAspectInterface* resourceAspect, UploadManager* uploadManager)
{
	auto diffuseTextureSid = material->m_textureSid[0];

	u32 slice = 0;
	auto it = m_entries.find(diffuseTextureSid);
	if (it == m_entries.end())
	{
		auto ptr = (u32*)m_freelist.insertEntry((u8*)m_freelistData.get(), sizeof(u32));
		if (ptr == nullptr)
			return false;

		auto textureIndex = ptr - m_freelistData.get();
		addTexture(textureIndex, diffuseTextureSid, resourceAspect, uploadManager);

		MaterialEntry entry;
		entry.slice = textureIndex;
		m_entries.insert(diffuseTextureSid, entry);

		slice = textureIndex;

	}
	else
	{
		slice = it->slice;
	}

	return true;
}