#pragma once

#include "d3d.h"
#include "d3dx12.h"
#include "Heap.h"

namespace d3d
{
	class Resource
	{
		Object<ID3D12Resource> m_res;
		D3D12_RESOURCE_STATES m_currentState;
		D3D12_RESOURCE_STATES m_originalState;

	public:
		Resource() :m_res(), m_currentState(), m_originalState() {}
		Resource(Object<ID3D12Resource> &&res, D3D12_RESOURCE_STATES state) :m_res(std::move(res)), m_currentState(state), m_originalState(state) {}
		Resource(const Resource&) = delete;
		Resource(Resource &&rhs) :m_res(std::move(rhs.m_res)), m_currentState(rhs.m_currentState), m_originalState(rhs.m_originalState) {}
		~Resource() { destroy(); }

		Resource& operator=(const Resource&) = delete;

		Resource& operator=(Resource &&rhs)
		{
			if (this != &rhs)
			{
				m_res = std::move(rhs.m_res);
				m_currentState = rhs.m_currentState;
				m_originalState = rhs.m_originalState;
			}
			return *this;
		}

		void destroy()
		{
			m_res.destroy();
		}

		D3D12_RESOURCE_STATES getCurrentState() const { return m_currentState; }
		D3D12_RESOURCE_STATES getOriginalState() const { return m_originalState; }

		ID3D12Resource* operator->()
		{
			return m_res.get();
		}

		operator ID3D12Resource*()
		{
			return m_res.get();
		}

		void SetName(const wchar_t* name)
		{
			m_res->SetName(name);
		}

		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress()
		{
			return m_res->GetGPUVirtualAddress();
		}

		D3D12_RESOURCE_DESC GetDesc()
		{
			return m_res->GetDesc();
		}

		ID3D12Resource* get() { return m_res.get(); }

		ID3D12Resource** getAddressOf()
		{
			return m_res.getAddressOf();
		}

		D3D12_RESOURCE_BARRIER barrierTransition(D3D12_RESOURCE_STATES newState, u32 subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
		{
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_res.get(), m_currentState, newState, subResource);
			m_currentState = newState;
			return barrier;
		}
	};
}