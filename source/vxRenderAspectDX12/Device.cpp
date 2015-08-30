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
			return false;

		hresult = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(m_factory.getAddressOf()));
		if (hresult != 0)
			return false;

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
		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.Height = 0;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Scaling = DXGI_SCALING_ASPECT_RATIO_STRETCH;
		desc.Stereo = 0;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.Width = 0;

		IDXGISwapChain1* swapChain = nullptr;
		auto hresult = m_factory->CreateSwapChainForHwnd(defaultQueue->get(), hwnd, &desc, nullptr, nullptr, &swapChain);
		if (hresult != 0)
			return false;

		hresult = swapChain->QueryInterface(IID_PPV_ARGS(m_swapChain.getAddressOf()));
		if (hresult != 0)
			return false;

		swapChain->Release();

		return true;
	}

	bool Device::initialize(const vx::Window &window, const D3D12_COMMAND_QUEUE_DESC &queueDesc, CommandQueue* defaultQueue)
	{
		if (!createDevice())
			return false;

		if (!defaultQueue->create(queueDesc, m_device.get()))
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

	void Device::swapBuffer()
	{
		m_swapChain->Present(1, 0);
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