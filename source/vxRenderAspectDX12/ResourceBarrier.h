#pragma once

struct D3D12_RESOURCE_BARRIER;

#include <d3d12.h>

namespace dx
{
	struct ResourceBarrier
	{
		D3D12_RESOURCE_BARRIER barrier;

		ResourceBarrier() :barrier() {}
		explicit ResourceBarrier(const D3D12_RESOURCE_BARRIER &b) :barrier(b) {}

		static inline ResourceBarrier Transition
			(
				ID3D12Resource* resource,
				D3D12_RESOURCE_STATES stateBefore,
				D3D12_RESOURCE_STATES stateAfter,
				UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
				D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE
			)
		{
			ResourceBarrier result;
			ZeroMemory(&result, sizeof(result));

			D3D12_RESOURCE_BARRIER &barrier = result.barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = flags;
			barrier.Transition.pResource = resource;
			barrier.Transition.StateBefore = stateBefore;
			barrier.Transition.StateAfter = stateAfter;
			barrier.Transition.Subresource = subresource;

			return result;
		}

		static inline ResourceBarrier Aliasing(
			_In_ ID3D12Resource* pResourceBefore,
			_In_ ID3D12Resource* pResourceAfter)
		{
			ResourceBarrier result;
			ZeroMemory(&result, sizeof(result));

			D3D12_RESOURCE_BARRIER &barrier = result.barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
			barrier.Aliasing.pResourceBefore = pResourceBefore;
			barrier.Aliasing.pResourceAfter = pResourceAfter;
			return result;
		}

		static inline ResourceBarrier UAV(
			_In_ ID3D12Resource* pResource)
		{
			ResourceBarrier result;
			ZeroMemory(&result, sizeof(result));

			D3D12_RESOURCE_BARRIER &barrier = result.barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			barrier.UAV.pResource = pResource;
			return result;
		}

		operator const D3D12_RESOURCE_BARRIER&() const { return barrier; }
	};
}