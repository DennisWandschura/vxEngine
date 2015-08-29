#pragma once

#include "d3d.h"

struct ID3D12GraphicsCommandList;
struct ID3D12CommandAllocator;
struct ID3D12PipelineState;
enum D3D12_COMMAND_LIST_TYPE;

namespace d3d
{
	class GraphicsCommandList
	{
		d3d::Object<ID3D12GraphicsCommandList> m_list;

	public:
		GraphicsCommandList();
		~GraphicsCommandList();

		bool create(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator *pCommandAllocator, ID3D12PipelineState *pInitialState = nullptr, u32 nodeMask = 1);
		void destroy();

		ID3D12GraphicsCommandList* operator->();

		ID3D12GraphicsCommandList* get() { return m_list.get(); }
	};
}