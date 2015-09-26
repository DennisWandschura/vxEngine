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
#include "Device.h"
#include <vxLib/Window.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include "CommandQueue.h"

namespace d3d
{
	typedef HRESULT(WINAPI *DXGIGetDebugInterfaceProc)(REFIID riid, void **ppDebug);

	Device::Device()
		:m_device(), 
		m_swapChain(),
		m_factory()
	{

	}

	Device::~Device()
	{
		shutdown();
	}

	bool Device::createDevice()
	{
		auto hresult = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(m_device.getAddressOf()));
		if (hresult != 0)
		{
			hresult = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_device.getAddressOf()));
		}
		
		if (hresult != 0)
			return false;

		hresult = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(m_factory.getAddressOf()));
		if (hresult != 0)
			return false;

		/*UINT i = 0;
		IDXGIAdapter1  * pAdapter;
		std::vector <IDXGIAdapter1 *> vAdapters;
		while (m_factory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
		{
			vAdapters.push_back(pAdapter);
			++i;
		}

		const char* COMPUTE_PREEMPTION_GRANULARITY[]=
		{
			"DXGI_COMPUTE_PREEMPTION_DMA_BUFFER_BOUNDARY",
			"DXGI_COMPUTE_PREEMPTION_DISPATCH_BOUNDARY",
			"DXGI_COMPUTE_PREEMPTION_THREAD_GROUP_BOUNDARY",
			"DXGI_COMPUTE_PREEMPTION_THREAD_BOUNDARY",
			"DXGI_COMPUTE_PREEMPTION_INSTRUCTION_BOUNDARY"
		};

		const char* GRAPHICS_PREEMPTION_GRANULARITY[] =
		{
			"DXGI_GRAPHICS_PREEMPTION_DMA_BUFFER_BOUNDARY",
			"DXGI_GRAPHICS_PREEMPTION_PRIMITIVE_BOUNDARY",
			"DXGI_GRAPHICS_PREEMPTION_TRIANGLE_BOUNDARY",
			"DXGI_GRAPHICS_PREEMPTION_PIXEL_BOUNDARY",
			"DXGI_GRAPHICS_PREEMPTION_INSTRUCTION_BOUNDARY"
		};

		for (auto &it : vAdapters)
		{
			IDXGIAdapter2* tmp = nullptr;
			auto hr = it->QueryInterface(IID_PPV_ARGS(&tmp));
			if(hr == 0)
			{
				DXGI_ADAPTER_DESC2 desc;
				tmp->GetDesc2(&desc);
				printf("%ws\n%s\n", desc.Description, COMPUTE_PREEMPTION_GRANULARITY[desc.ComputePreemptionGranularity]);
				printf("%s\n", GRAPHICS_PREEMPTION_GRANULARITY[desc.GraphicsPreemptionGranularity]);
				printf("\n");
			}
		}*/

		return true;
	}

	bool Device::createSwapChain(const vx::Window &window, CommandQueue* defaultQueue)
	{
		auto hwnd = window.getHwnd();
		auto resolution = window.getSize();

		DXGI_SWAP_CHAIN_DESC1 desc;
		desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		desc.BufferCount = 2;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Height = 0;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Scaling = DXGI_SCALING_ASPECT_RATIO_STRETCH;
		desc.Stereo = 0;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.Width = 0;

		IDXGISwapChain1* swapChain = nullptr;
		HRESULT hresult = m_factory->CreateSwapChainForHwnd(defaultQueue->get(), hwnd, &desc, nullptr, nullptr, &swapChain);
		if (hresult != 0)
			return false;

		hresult = swapChain->QueryInterface(IID_PPV_ARGS(m_swapChain.getAddressOf()));
		if (hresult != 0)
			return false;

		swapChain->Release();

		return true;
	}

	bool Device::initialize(const vx::Window &window, const D3D12_COMMAND_QUEUE_DESC &queueDesc, u32 queueCapacity, CommandQueue* defaultQueue)
	{
		if (!createDevice())
			return false;

		if (!defaultQueue->create(queueDesc, m_device.get(), queueCapacity))
			return false;

		if (!createSwapChain(window, defaultQueue))
			return false;

		return true;
	}

	void Device::shutdown()
	{
		m_swapChain.destroy();

		m_factory.destroy();
		m_device.destroy();
	}

	void Device::present()
	{
		DXGI_PRESENT_PARAMETERS params;
		params.DirtyRectsCount = 0;
		params.pDirtyRects = nullptr;
		params.pScrollOffset = nullptr;
		params.pScrollRect = nullptr;

		m_swapChain->Present1(1, 0, &params);
	}

	u32 Device::getCurrentBackBufferIndex()
	{
		return m_swapChain->GetCurrentBackBufferIndex();
	}

	bool Device::getBuffer(u32 index, const _GUID &riid, void **ppSurface)
	{
		return (m_swapChain->GetBuffer(index, riid, ppSurface) == 0);
	}
}