#include "Device.h"
#include <vxLib/Window.h>
#include <d3d12.h>
#include <dxgi1_4.h>

namespace d3d
{
	Device::Device()
		:m_commandQueue(),
		m_swapChain(),
		m_fence(),
		m_currentFence(0),
		m_event(nullptr),
		m_device(),
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

		hresult = CreateDXGIFactory1(IID_PPV_ARGS(m_factory.getAddressOf()));
		if (hresult != 0)
			return false;

		hresult = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.getAddressOf()));
		if (hresult != 0)
			return false;

		m_event = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);

		return true;
	}

	bool Device::createCommandQueue()
	{
		D3D12_COMMAND_QUEUE_DESC desc;
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		desc.Priority = 0;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		auto hresult = m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(m_commandQueue.getAddressOf()));
		if (hresult != 0)
			return false;

		return true;
	}

	bool Device::createSwapChain(const vx::Window &window)
	{
		DXGI_SWAP_CHAIN_DESC descSwapChain;
		ZeroMemory(&descSwapChain, sizeof(descSwapChain));
		descSwapChain.BufferCount = 2;
		descSwapChain.BufferDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		descSwapChain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		descSwapChain.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		descSwapChain.OutputWindow = window.getHwnd();
		descSwapChain.SampleDesc.Count = 1;
		descSwapChain.Windowed = 1;

		IDXGISwapChain* swapChain;
		auto hresult = m_factory->CreateSwapChain(m_commandQueue.get(), &descSwapChain, &swapChain);

		if (hresult != 0)
			return false;

		hresult = swapChain->QueryInterface(IID_PPV_ARGS(m_swapChain.getAddressOf()));
		if (hresult != 0)
			return false;

		return true;
	}

	bool Device::initialize(const vx::Window &window)
	{
		if (!createDevice())
			return false;

		if (!createCommandQueue())
			return false;

		if (!createSwapChain(window))
			return false;

		return true;
	}

	void Device::shutdown()
	{
		m_swapChain.destroy();
		m_commandQueue.destroy();
		m_factory.destroy();
		m_device.destroy();
	}

	void Device::executeCommandLists(u32 count, ID3D12CommandList** lists)
	{
		m_commandQueue->ExecuteCommandLists(count, lists);
	}

	void Device::swapBuffer()
	{
		m_swapChain->Present(1, 0);
	}

	void Device::waitForGpu()
	{
		const u64 fence = m_currentFence++;
		m_commandQueue->Signal(m_fence.get(), fence);

		//
		// Let the previous frame finish before continuing
		//

		if (m_fence->GetCompletedValue() < fence)
		{
			m_fence->SetEventOnCompletion(fence, m_event);
			WaitForSingleObject(m_event, INFINITE);
		}
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