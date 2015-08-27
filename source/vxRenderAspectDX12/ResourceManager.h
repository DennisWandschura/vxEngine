#pragma once

struct ID3D12Resource;
enum D3D12_RESOURCE_STATES;
struct D3D12_CONSTANT_BUFFER_VIEW_DESC;
struct D3D12_SHADER_RESOURCE_VIEW_DESC;

#include "d3d.h"
#include <vxLib/Container/sorted_vector.h>
#include <vxLib/StringID.h>
#include "Heap.h"

namespace d3d
{
	class Device;

	struct ResourceView;

	class ResourceManager
	{
		vx::sorted_vector<vx::StringID, d3d::Object<ID3D12Resource>> m_buffers;
		vx::sorted_vector<vx::StringID, d3d::Object<ID3D12Resource>> m_textures;
		vx::sorted_vector<vx::StringID, d3d::Object<ID3D12Resource>> m_texturesRtDs;

		vx::sorted_vector<vx::StringID, D3D12_CONSTANT_BUFFER_VIEW_DESC> m_cbufferViews;
		vx::sorted_vector<vx::StringID, D3D12_SHADER_RESOURCE_VIEW_DESC> m_srViews;
		vx::sorted_vector<vx::StringID, ResourceView> m_resourceViews;

		d3d::Heap m_bufferHeap;
		d3d::Heap m_textureHeap;
		d3d::Heap m_rtDsHeap;

		bool createHeaps(u64 heapSizeBuffer, u64 heapSizeTexture, u64 heapSizeRtDs, ID3D12Device* device);

	public:
		ResourceManager();
		~ResourceManager();

		bool initializeHeaps(u64 heapSizeBuffer, u64 heapSizeTexture, u64 heapSizeRtvDsv, ID3D12Device* device);
		void shutdown();

		ID3D12Resource* createBuffer(const wchar_t* id, u64 size, u32 state);
		ID3D12Resource* createTexture(const wchar_t* id, const CreateResourceDesc &desc);
		ID3D12Resource* createTextureRtDs(const wchar_t* id, const CreateResourceDesc &desc);

		void insertConstantBufferView(const char* id, const D3D12_CONSTANT_BUFFER_VIEW_DESC &desc);
		void insertShaderResourceView(const char* id, const D3D12_SHADER_RESOURCE_VIEW_DESC &desc);
		void insertResourceView(const char* id, const ResourceView &desc);

		ID3D12Resource* getBuffer(const wchar_t* id);
		ID3D12Resource* getTexture(const wchar_t* id);
		ID3D12Resource* getTextureRtDs(const wchar_t* id);

		const ResourceView* getResourceView(const char* id) const;
		ResourceView* getResourceView(const char* id);

		const D3D12_CONSTANT_BUFFER_VIEW_DESC* getConstantBufferView(const char* id) const;
		const D3D12_SHADER_RESOURCE_VIEW_DESC* getShaderResourceView(const char* id) const;
	};
}