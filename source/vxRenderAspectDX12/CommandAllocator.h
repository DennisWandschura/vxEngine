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

enum D3D12_COMMAND_LIST_TYPE;
struct ID3D12CommandAllocator;

#include "d3d.h"

namespace d3d
{
	class CommandAllocator
	{
		d3d::Object<ID3D12CommandAllocator> m_commandAllocator;

	public:
		CommandAllocator();
		CommandAllocator(const CommandAllocator&) = delete;
		CommandAllocator(CommandAllocator &&rhs);
		~CommandAllocator();

		bool create(D3D12_COMMAND_LIST_TYPE type, ID3D12Device* device);
		void destroy();

		ID3D12CommandAllocator* get() { return m_commandAllocator.get(); }
		ID3D12CommandAllocator* operator->() { return m_commandAllocator.get(); }
	};
}