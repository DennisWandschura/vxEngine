#pragma once

/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

struct ID3D12Resource;
enum D3D12_RESOURCE_STATES;
struct D3D12_CONSTANT_BUFFER_VIEW_DESC;
struct D3D12_SHADER_RESOURCE_VIEW_DESC;
class Logfile;

#include "d3d.h"
#include <vxLib/Container/sorted_vector.h>
#include <vxLib/StringID.h>
#include "Resource.h"

namespace d3d
{
	class Device;

	struct ResourceView;

	class ResourceManager
	{
		vx::sorted_vector<vx::StringID, Resource> m_buffers;
		vx::sorted_vector<vx::StringID, Resource> m_textures;
		vx::sorted_vector<vx::StringID, Resource> m_texturesRtDs;

		vx::sorted_vector<vx::StringID, D3D12_CONSTANT_BUFFER_VIEW_DESC> m_cbufferViews;
		vx::sorted_vector<vx::StringID, D3D12_SHADER_RESOURCE_VIEW_DESC> m_srViews;
		vx::sorted_vector<vx::StringID, ResourceView> m_resourceViews;

		d3d::Heap m_bufferHeap;
		d3d::Heap m_textureHeap;
		d3d::Heap m_rtDsHeap;
		Logfile* m_logfile;

		bool createHeaps(u64 heapSizeBuffer, u64 heapSizeTexture, u64 heapSizeRtDs, ID3D12Device* device);

	public:
		ResourceManager();
		~ResourceManager();

		bool initializeHeaps(u64 heapSizeBuffer, u64 heapSizeTexture, u64 heapSizeRtvDsv, ID3D12Device* device, Logfile* logfile);
		void shutdown();

		Resource* createBuffer(const wchar_t* id, u64 size, u32 state, u32 flags = 0);
		Resource* createTexture(const wchar_t* id, const CreateResourceDesc &desc);
		Resource* createTextureRtDs(const wchar_t* id, const CreateResourceDesc &desc);

		void insertConstantBufferView(const char* id, const D3D12_CONSTANT_BUFFER_VIEW_DESC &desc);
		void insertShaderResourceView(const char* id, const D3D12_SHADER_RESOURCE_VIEW_DESC &desc);
		void insertResourceView(const char* id, const ResourceView &desc);

		Resource* getBuffer(const wchar_t* id);
		Resource* getBuffer(const vx::StringID &sid);
		Resource* getTexture(const wchar_t* id);
		Resource* getTexture(const vx::StringID &sid);
		Resource* getTextureRtDs(const wchar_t* id);
		Resource* getTextureRtDs(const vx::StringID &sid);

		const ResourceView* getResourceView(const char* id) const;
		ResourceView* getResourceView(const char* id);

		const D3D12_CONSTANT_BUFFER_VIEW_DESC* getConstantBufferView(const char* id) const;
		const D3D12_SHADER_RESOURCE_VIEW_DESC* getShaderResourceView(const char* id) const;
	};
}