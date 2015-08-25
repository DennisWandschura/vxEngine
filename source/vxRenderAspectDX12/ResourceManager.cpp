#include "ResourceManager.h"
#include "Heap.h"
#include <d3d12.h>
#include "ResourceView.h"

namespace d3d
{
	ResourceManager::ResourceManager()
		:m_buffers(),
		m_textures()
	{

	}

	ResourceManager::~ResourceManager()
	{

	}

	bool ResourceManager::initialize()
	{
		return true;
	}

	void ResourceManager::shutdown()
	{
		m_buffers.clear();
		m_textures.clear();
	}

	bool ResourceManager::createBuffer(const char* id, const BufferDesc &desc)
	{
		Buffer buffer;

		BufferResourceDesc resDesc;
		resDesc.size = desc.size;
		resDesc.resource = buffer.getAddressOf();
		resDesc.state = desc.state;

		bool result = false;
		if (desc.heap->createResourceBuffer(resDesc))
		{
			result = true;
			auto sid = vx::make_sid(id);
			m_buffers.insert(std::move(sid), std::move(buffer));
		}

		return result;
	}

	void ResourceManager::insertBuffer(const char* id, d3d::Object<ID3D12Resource> &&buffer)
	{
		auto sid = vx::make_sid(id);
		m_buffers.insert(std::move(sid), std::move(buffer));
	}

	void ResourceManager::insertTexture(const char* id, d3d::Object<ID3D12Resource> &&texture)
	{
		auto sid = vx::make_sid(id);
		m_textures.insert(std::move(sid), std::move(texture));
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

	ID3D12Resource* ResourceManager::getBuffer(const char* id)
	{
		auto sid = vx::make_sid(id);
		auto it = m_buffers.find(sid);

		return (it != m_buffers.end()) ? it->get() : nullptr;
	}

	ID3D12Resource* ResourceManager::getTexture(const char* id)
	{
		auto sid = vx::make_sid(id);
		auto it = m_textures.find(sid);

		return (it != m_textures.end()) ? it->get() : nullptr;
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