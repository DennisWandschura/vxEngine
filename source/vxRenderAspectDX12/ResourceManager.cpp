#include "ResourceManager.h"
#include "Heap.h"
#include <d3d12.h>
#include "ResourceView.h"
#include <vxEngineLib/Logfile.h>

namespace d3d
{
	namespace ResourceManagerCpp
	{
		template<typename T>
		void destroyResourceArray(T* src)
		{
			for (auto &it : *src)
			{
				it.destroy();
			}
			src->clear();
		}
	}

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

	bool ResourceManager::initializeHeaps(u64 heapSizeBuffer, u64 heapSizeTexture, u64 heapSizeRtDs, ID3D12Device* device, Logfile* logfile)
	{
		if (!createHeaps(heapSizeBuffer, heapSizeTexture, heapSizeRtDs, device))
			return false;

		m_logfile = logfile;

		return true;
	}

	void ResourceManager::shutdown()
	{
		ResourceManagerCpp::destroyResourceArray(&m_buffers);
		ResourceManagerCpp::destroyResourceArray(&m_textures);
		ResourceManagerCpp::destroyResourceArray(&m_texturesRtDs);

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
				auto sid = vx::make_sid(id);
				auto it = m_buffers.insert(std::move(sid), Resource(std::move(buffer), resDesc.state));
				ptr = (&*it);

				ptr->SetName(id);

				char buffer[64];
				auto sz = sprintf(buffer, "Buffer %ws %p\n", id, (*ptr).get());
				if (sz < 64)
				{
					m_logfile->append(buffer, sz);
				}
				else
				{
					puts("");
				}
				//printf("Buffer %ws %p\n", id, (*ptr).get());
			}
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
				auto sid = vx::make_sid(id);

				auto it = m_textures.insert(std::move(sid), Resource(std::move(texture), desc.state));
				ptr = &(*it);
				ptr->SetName(id);

				//printf("Texture %ws %p\n", id, (*ptr).get());
			}
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
				auto sid = vx::make_sid(id);
				auto it = m_texturesRtDs.insert(std::move(sid), Resource(std::move(texture), desc.state));
				ptr = (&*it);
				ptr->SetName(id);

				//printf("TextureRtDs %ws %p\n", id, (*ptr).get());
			}
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
		auto it = m_buffers.find(sid);

		return (it != m_buffers.end()) ? &(*it) : nullptr;
	}

	Resource* ResourceManager::getTexture(const wchar_t* id)
	{
		auto sid = vx::make_sid(id);
		return getTexture(sid);
	}

	Resource* ResourceManager::getTexture(const vx::StringID &sid)
	{
		auto it = m_textures.find(sid);

		return (it != m_textures.end()) ? &(*it) : nullptr;
	}

	Resource* ResourceManager::getTextureRtDs(const wchar_t* id)
	{
		auto sid = vx::make_sid(id);
		return getTextureRtDs(sid);
	}

	Resource* ResourceManager::getTextureRtDs(const vx::StringID &sid)
	{
		auto it = m_texturesRtDs.find(sid);

		return (it != m_texturesRtDs.end()) ? &(*it) : nullptr;
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