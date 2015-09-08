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

#include "d3d.h"
#include <vxEngineLib/Graphics/CommandList.h>

struct ID3D12GraphicsCommandList;
struct ID3D12CommandAllocator;
struct ID3D12PipelineState;
enum D3D12_COMMAND_LIST_TYPE;

namespace d3d
{
	class GraphicsCommandList : public ::Graphics::CommandList
	{
		d3d::Object<ID3D12GraphicsCommandList> m_list;

	public:
		GraphicsCommandList();
		GraphicsCommandList(GraphicsCommandList &&rhs);
		~GraphicsCommandList();

		bool create(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator *pCommandAllocator, ID3D12PipelineState *pInitialState = nullptr, u32 nodeMask = 1);
		void destroy();

		ID3D12GraphicsCommandList* operator->();

		ID3D12GraphicsCommandList* get() { return m_list.get(); }
	};
}