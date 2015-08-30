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

struct IDXGIDebug;
struct IDXGIInfoQueue;
struct ID3D12InfoQueue;
struct ID3D12Debug;

#include "d3d.h"
#include <vxLib/Allocator/StackAllocator.h>

namespace d3d
{
	class Debug
	{
		d3d::Object<ID3D12InfoQueue> m_infoQueue;
		d3d::Object<IDXGIInfoQueue> m_dxgiInfoQueue;
		vx::StackAllocator m_scratchAllocator;
		d3d::Object<ID3D12Debug> m_debug;
		d3d::Object<IDXGIDebug> m_dxgiDebug;
		void* m_dxgidebugDllHandle;

	public:
		Debug();
		~Debug();

		bool initializeDebugMode();
		bool initialize(vx::StackAllocator* allocator, ID3D12Device* device);
		void shutdownDevice();
		void shutdown();

		void printDebugMessages();

		void reportLiveObjects();
	};
}