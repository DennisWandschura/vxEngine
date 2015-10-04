#include "ResourceManager.h"
#include "Heap.h"
#include <d3d12.h>
#include "ResourceView.h"
#include <vxEngineLib/Logfile.h>

namespace d3d
{
	ResourceManager::ResourceManager()
		:m_buffers(),
		m_textures(),
		m_logfile(nullptr)
	{

	}

	ResourceManager::~ResourceManager()
	{

	}

	bool ResourceManager::createHeaps(u64 heapSizeBuffer, u64 heapSizeTexture, u64 heapSizeRtDs, ID3D12Device* device)
	{
		D3D12_HEAP_DESC desc;
		desc.Alignment = 64 KBYTE;
		desc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
		desc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		desc.Properties.CreationNodeMask = 1;
		desc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		desc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
		desc.Properties.VisibleNodeMask = 1;
		desc.SizeInBytes = heapSizeBuffer;
		if (!m_bufferHeap.create(desc, device))
			return false;

		m_bufferHeap.setName(L"ResourceManagerHeapBuffer");

		desc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
		desc.SizeInBytes = heapSizeTexture;
		if (!m_textureHeap.create(desc, device))
			return false;

		m_textureHeap.setName(L"ResourceManagerHeapTexture");

		desc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES;
		desc.SizeInBytes = heapSizeRtDs;
		if (!m_rtDsHeap.create(desc, device))
			return false;

		m_rtDsHeap.setName(L"ResourceManagerHeapRtDs");

		printf("Buffers: %.4f MB\n", heapSizeBuffer / 1024.0 / 1024.0);
		printf("Textures: %.4f MB\n", heapSizeTexture / 1024.0 / 1024.0);
		printf("Render Targets: %.4f MB\n", heapSizeRtDs / 1024.0 / 1024.0);

		return true;
	}

	bool ResourceManager::initializeHeaps(u64 heapSizeBuffer, u32 bufferCount, u64 heapSizeTexture, u32 textureCount, u64 heapSizeRtDs, u32 rtDsCount, ID3D12Device* device, Logfile* logfile)
	{
		if (!createHeaps(heapSizeBuffer, heapSizeTexture, heapSizeRtDs, device))
			return false;

		m_buffers = std::make_unique<Resource[]>(bufferCount);
		m_bufferIndices = std::make_unique<u32[]>(bufferCount);
		m_freelistBuffer.create((u8*)m_bufferIndices.get(), bufferCount, sizeof(u32));
		m_sortedBuffers.reserve(bufferCount);

		m_textures = std::make_unique<Resource[]>(textureCount);
		m_textureIndices = std::make_unique<u32[]>(textureCount);
		m_freelistTextures.create((u8*)m_textureIndices.get(), textureCount, sizeof(u32));
		m_sortedTextures.reserve(textureCount);

		m_texturesRtDs = std::make_unique<Resource[]>(rtDsCount);
		m_rtDvIndices = std::make_unique<u32[]>(rtDsCount);
		m_freelistRtDv.create((u8*)m_rtDvIndices.get(), rtDsCount, sizeof(u32));
		m_sortedTexturesRtDs.reserve(rtDsCount);

		m_logfile = logfile;

		return true;
	}

	void ResourceManager::shutdown()
	{
		for (auto &it : m_sortedBuffers)
		{
			m_buffers[it].destroy();
		}
		m_sortedBuffers.clear();

		for (auto &it : m_sortedTextures)
		{
			m_textures[it].destroy();
		}
		m_sortedTextures.clear();

		for (auto &it : m_sortedTexturesRtDs)
		{
			m_texturesRtDs[it].destroy();
		}
		m_sortedTexturesRtDs.clear();

		m_bufferHeap.destroy();
		m_textureHeap.destroy();
		m_rtDsHeap.destroy();
	}

	Resource* ResourceManager::createBuffer(const wchar_t* id, u64 size, u32 state, u32 flags)
	{
		auto ptr = getBuffer(id);
		if (ptr == nullptr)
		{
			Object<ID3D12Resource> buffer;
			HeapCreateBufferResourceDesc resDesc;
			resDesc.size = size;
			resDesc.resource = buffer.getAddressOf();
			resDesc.state = (D3D12_RESOURCE_STATES)state;
			resDesc.flags = (D3D12_RESOURCE_FLAGS)flags;

			if (m_bufferHeap.createResourceBuffer(resDesc))
			{
				auto ptrFreelist = (u32*)m_freelistBuffer.insertEntry((u8*)m_bufferIndices.get(), sizeof(u32));
				u32 index = ptrFreelist - m_bufferIndices.get();

				auto sid = vx::make_sid(id);
				auto resource = Resource(std::move(buffer), resDesc.state);
				resource->SetName(id);

				m_sortedBuffers.insert(sid, index);

				m_buffers[index] = std::move(resource);
				ptr = &m_buffers[index];

				/*char buffer[64];
				auto sz = snprintf(buffer, 64, "Buffer %ws %p\n", id, (*ptr).get());
				if (sz < 64)
				{
					//m_logfile->append(buffer, sz);
				}
				else
				{
				}*/
			}
		}
		else
		{
			ptr = nullptr;
		}

		return ptr;
	}

	Resource* ResourceManager::createTexture(const wchar_t* id, const CreateResourceDesc &desc)
	{
		auto ptr = getTexture(id);
		if (ptr == nullptr)
		{
			d3d::Object<ID3D12Resource> texture;
			d3d::HeapCreateResourceDesc resDesc;
			resDesc.desc = desc;
			resDesc.resource = texture.getAddressOf();
			if (m_textureHeap.createResource(resDesc))
			{
				auto ptrFreelist = (u32*)m_freelistTextures.insertEntry((u8*)m_textureIndices.get(), sizeof(u32));
				u32 index = ptrFreelist - m_textureIndices.get();

				auto resource = Resource(std::move(texture), desc.state);
				resource->SetName(id);

				m_textures[index] = std::move(resource);
				ptr = &m_textures[index];

				auto sid = vx::make_sid(id);
				m_sortedTextures.insert(std::move(sid), index);
			}
			else
			{
				printf("ResourceManager::createTexture: error creating texture\n");
			}
		}
		else
		{
			printf("ResourceManager::createTexture: error\n");
			ptr = nullptr;
		}

		return ptr;
	}

	Resource* ResourceManager::createTextureRtDs(const wchar_t* id, const CreateResourceDesc &desc)
	{
		auto ptr = getTextureRtDs(id);
		if (ptr == nullptr)
		{
			d3d::Object<ID3D12Resource> texture;
			d3d::HeapCreateResourceDesc resDesc;
			resDesc.desc = desc;
			resDesc.resource = texture.getAddressOf();
			if (m_rtDsHeap.createResource(resDesc))
			{
				auto ptrFreelist = (u32*)m_freelistRtDv.insertEntry((u8*)m_rtDvIndices.get(), sizeof(u32));
				u32 index = ptrFreelist - m_rtDvIndices.get();

				auto resource = Resource(std::move(texture), desc.state);
				resource->SetName(id);

				m_texturesRtDs[index] = std::move(resource);
				ptr = &m_texturesRtDs[index];

				auto sid = vx::make_sid(id);
				m_sortedTexturesRtDs.insert(sid, index);
			}
		}
		else
		{
			ptr = nullptr;
		}

		return ptr;
	}

	void ResourceManager::insertConstantBufferView(const char* id, const D3D12_CONSTANT_BUFFER_VIEW_DESC &desc)
	{
		auto sid = vx::make_sid(id);
		m_cbufferViews.insert(sid, desc);
	}

	void ResourceManager::insertShaderResourceView(const char* id, const D3D12_SHADER_RESOURCE_VIEW_DESC &desc)
	{
		auto sid = vx::make_sid(id);
		m_srViews.insert(sid, desc);
	}

	void ResourceManager::insertResourceView(const char* id, const ResourceView &desc)
	{
		auto sid = vx::make_sid(id);
		m_resourceViews.insert(sid, desc);
	}

	Resource* ResourceManager::getBuffer(const wchar_t* id)
	{
		auto sid = vx::make_sid(id);
		return getBuffer(sid);
	}

	Resource* ResourceManager::getBuffer(const vx::StringID &sid)
	{
		auto it = m_sortedBuffers.find(sid);

		return (it != m_sortedBuffers.end()) ? &m_buffers[*it] : nullptr;
	}

	Resource* ResourceManager::getTexture(const wchar_t* id)
	{
		auto sid = vx::make_sid(id);
		return getTexture(sid);
	}

	Resource* ResourceManager::getTexture(const vx::StringID &sid)
	{
		auto it = m_sortedTextures.find(sid);

		return (it != m_sortedTextures.end()) ? &m_textures[*it] : nullptr;
	}

	Resource* ResourceManager::getTextureRtDs(const wchar_t* id)
	{
		auto sid = vx::make_sid(id);
		return getTextureRtDs(sid);
	}

	Resource* ResourceManager::getTextureRtDs(const vx::StringID &sid)
	{
		auto it = m_sortedTexturesRtDs.find(sid);

		return (it != m_sortedTexturesRtDs.end()) ? &m_texturesRtDs[*it] : nullptr;
	}

	const ResourceView* ResourceManager::getResourceView(const char* id) const
	{
		auto sid = vx::make_sid(id);
		auto it = m_resourceViews.find(sid);

		return (it != m_resourceViews.end()) ? &*it : nullptr;
	}

	ResourceView* ResourceManager::getResourceView(const char* id)
	{
		auto sid = vx::make_sid(id);
		auto it = m_resourceViews.find(sid);

		return (it != m_resourceViews.end()) ? &*it : nullptr;
	}

	const D3D12_CONSTANT_BUFFER_VIEW_DESC* ResourceManager::getConstantBufferView(const char* id) const
	{
		auto sid = vx::make_sid(id);
		auto it = m_cbufferViews.find(sid);

		return (it != m_cbufferViews.end()) ? &*it : nullptr;
	}

	const D3D12_SHADER_RESOURCE_VIEW_DESC* ResourceManager::getShaderResourceView(const char* id) const
	{
		auto sid = vx::make_sid(id);
		auto it = m_srViews.find(sid);

		return (it != m_srViews.end()) ? &*it : nullptr;
	}
}