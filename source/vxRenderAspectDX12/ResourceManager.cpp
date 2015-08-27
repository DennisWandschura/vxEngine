#include "ResourceManager.h"
#include "Heap.h"
#include <d3d12.h>
#include "ResourceView.h"

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
		m_textures()
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

		return true;
	}

	bool ResourceManager::initializeHeaps(u64 heapSizeBuffer, u64 heapSizeTexture, u64 heapSizeRtDs, ID3D12Device* device)
	{
		if (!createHeaps(heapSizeBuffer, heapSizeTexture, heapSizeRtDs, device))
			return false;

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

	ID3D12Resource* ResourceManager::createBuffer(const wchar_t* id, u64 size, u32 state)
	{
		auto ptr = getBuffer(id);
		if (ptr == nullptr)
		{
			Object<ID3D12Resource> buffer;
			HeapCreateBufferResourceDesc resDesc;
			resDesc.size = size;
			resDesc.resource = buffer.getAddressOf();
			resDesc.state = (D3D12_RESOURCE_STATES)state;

			if (m_bufferHeap.createResourceBuffer(resDesc))
			{
				auto sid = vx::make_sid(id);
				auto it = m_buffers.insert(std::move(sid), std::move(buffer));
				ptr = it->get();

				ptr->SetName(id);
			}
		}

		return ptr;
	}

	ID3D12Resource* ResourceManager::createTexture(const wchar_t* id, const CreateResourceDesc &desc)
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
				auto it = m_textures.insert(std::move(sid), std::move(texture));
				ptr = it->get();
				ptr->SetName(id);
			}
		}

		return ptr;
	}

	ID3D12Resource* ResourceManager::createTextureRtDs(const wchar_t* id, const CreateResourceDesc &desc)
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
				auto it = m_texturesRtDs.insert(std::move(sid), std::move(texture));
				ptr = it->get();
				ptr->SetName(id);
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

	ID3D12Resource* ResourceManager::getBuffer(const wchar_t* id)
	{
		auto sid = vx::make_sid(id);
		auto it = m_buffers.find(sid);

		return (it != m_buffers.end()) ? it->get() : nullptr;
	}

	ID3D12Resource* ResourceManager::getTexture(const wchar_t* id)
	{
		auto sid = vx::make_sid(id);
		auto it = m_textures.find(sid);

		return (it != m_textures.end()) ? it->get() : nullptr;
	}

	ID3D12Resource* ResourceManager::getTextureRtDs(const wchar_t* id)
	{
		auto sid = vx::make_sid(id);
		auto it = m_texturesRtDs.find(sid);

		return (it != m_texturesRtDs.end()) ? it->get() : nullptr;
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