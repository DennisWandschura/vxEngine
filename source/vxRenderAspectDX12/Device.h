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

namespace vx
{
	class Window;
}

struct IDXGISwapChain3;
struct ID3D12Device;
struct IDXGIFactory4;
struct D3D12_COMMAND_QUEUE_DESC;

#include <vxLib/types.h>
#include "d3d.h"

namespace d3d
{
	class CommandQueue;

	class Device
	{
		Object<ID3D12Device> m_device;
		Object<IDXGISwapChain3> m_swapChain;
		Object<IDXGIFactory4> m_factory;

		bool createDevice();
		bool createSwapChain(const vx::Window &window, CommandQueue* defaultQueue);

	public:
		Device();
		~Device();

		bool initialize(const vx::Window &window, const D3D12_COMMAND_QUEUE_DESC &queueDesc, u32 queueCapacity, CommandQueue* defaultQueue);
		void shutdown();

		void present();

		ID3D12Device* operator->()
		{
			return m_device.get();
		}

		IDXGISwapChain3* getSwapChain() { return m_swapChain.get(); }
		ID3D12Device* getDevice() { return m_device.get(); }

		u32 getCurrentBackBufferIndex();
		bool getBuffer(u32 index, const _GUID &riid, void **ppSurface);
	};
}