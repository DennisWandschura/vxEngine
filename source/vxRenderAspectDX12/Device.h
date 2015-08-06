#pragma once

namespace vx
{
	class Window;
}

struct ID3D12CommandQueue;
struct IDXGISwapChain3;
struct ID3D12Device;
struct IDXGIFactory4;
struct ID3D12CommandList;
struct _GUID;
struct ID3D12Fence;

#include <vxLib/types.h>
#include "d3d.h"

namespace d3d
{
	class Device
	{
		Object<ID3D12CommandQueue> m_commandQueue;
		Object<IDXGISwapChain3> m_swapChain;
		Object<ID3D12Fence> m_fence;
		u64 m_currentFence;
		void* m_event;
		Object<ID3D12Device> m_device;
		Object<IDXGIFactory4> m_factory;

		bool createDevice();
		bool createCommandQueue();
		bool createSwapChain(const vx::Window &window);

	public:
		Device();
		~Device();

		bool initialize(const vx::Window &window);
		void shutdown();

		void executeCommandLists(u32 count, ID3D12CommandList** lists);

		void swapBuffer();
		void waitForGpu();

		IDXGISwapChain3* getSwapChain() { return m_swapChain.get(); }
		ID3D12Device* getDevice() { return m_device.get(); }
		ID3D12CommandQueue* getCommandQueue() { return m_commandQueue.get(); }

		u32 getCurrentBackBufferIndex();
		bool getBuffer(u32 index, const _GUID &riid, void **ppSurface);
	};
}