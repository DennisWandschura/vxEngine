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