#pragma once

struct ID3D12Resource;
enum D3D12_RESOURCE_STATES;
struct D3D12_CONSTANT_BUFFER_VIEW_DESC;
struct D3D12_SHADER_RESOURCE_VIEW_DESC;

#include <vxLib/Container/sorted_vector.h>
#include "d3d.h"
#include <vxLib/StringID.h>

namespace d3d
{
	class Device;
	class Heap;

	struct ResourceView;

	struct BufferDesc
	{
		u64 offset;
		u64 size; 
		D3D12_RESOURCE_STATES state;
		Heap* heap;
	};

	class ResourceManager
	{
		vx::sorted_vector<vx::StringID, d3d::Object<ID3D12Resource>> m_buffers;
		vx::sorted_vector<vx::StringID, d3d::Object<ID3D12Resource>> m_textures;

		vx::sorted_vector<vx::StringID, D3D12_CONSTANT_BUFFER_VIEW_DESC> m_cbufferViews;
		vx::sorted_vector<vx::StringID, D3D12_SHADER_RESOURCE_VIEW_DESC> m_srViews;
		vx::sorted_vector<vx::StringID, ResourceView> m_resourceViews;

	public:
		ResourceManager();
		~ResourceManager();

		bool initialize();
		void shutdown();

		bool createBuffer(const char* id, const BufferDesc &desc);
		void insertBuffer(const char* id, d3d::Object<ID3D12Resource> &&buffer);

		void insertTexture(const char* id, d3d::Object<ID3D12Resource> &&texture);

		void insertConstantBufferView(const char* id, const D3D12_CONSTANT_BUFFER_VIEW_DESC &desc);
		void insertShaderResourceView(const char* id, const D3D12_SHADER_RESOURCE_VIEW_DESC &desc);
		void insertResourceView(const char* id, const ResourceView &desc);

		ID3D12Resource* getBuffer(const char* id);
		ID3D12Resource* getTexture(const char* id);

		const ResourceView* getResourceView(const char* id) const;
		ResourceView* getResourceView(const char* id);

		const D3D12_CONSTANT_BUFFER_VIEW_DESC* getConstantBufferView(const char* id) const;
		const D3D12_SHADER_RESOURCE_VIEW_DESC* getShaderResourceView(const char* id) const;
	};
}