#pragma once

#include <d3d12.h>
#include <cstdio>

inline void printError(HRESULT error)
{
	switch (error)
	{
	case E_FAIL:
		puts("E_FAIL");
		break;
	case E_INVALIDARG:
		puts("E_INVALIDARG");
		break;
	case E_OUTOFMEMORY:
		puts("E_OUTOFMEMORY");
		break;
	case S_FALSE:
		puts("S_FALSE");
		break;
	case DXGI_ERROR_DEVICE_HUNG:
		puts("DXGI_ERROR_DEVICE_HUNG");
		break;
	case DXGI_ERROR_DEVICE_REMOVED:
		puts("DXGI_ERROR_DEVICE_REMOVED");
		break;
	case DXGI_ERROR_DEVICE_RESET:
		puts("DXGI_ERROR_DEVICE_RESET");
		break;
	case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
		puts("DXGI_ERROR_DRIVER_INTERNAL_ERROR");
		break;
	case DXGI_ERROR_FRAME_STATISTICS_DISJOINT:
		puts("DXGI_ERROR_FRAME_STATISTICS_DISJOINT");
		break;
	case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE:
		puts("DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE");
		break;
	case DXGI_ERROR_INVALID_CALL:
		puts("DXGI_ERROR_INVALID_CALL");
		break;
	case DXGI_ERROR_MORE_DATA:
		puts("DXGI_ERROR_MORE_DATA");
		break;
	case DXGI_ERROR_NONEXCLUSIVE:
		puts("DXGI_ERROR_NONEXCLUSIVE");
		break;
	case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE:
		puts("DXGI_ERROR_NOT_CURRENTLY_AVAILABLE");
		break;
	case DXGI_ERROR_NOT_FOUND:
		puts("DXGI_ERROR_NOT_FOUND");
		break;
	case DXGI_ERROR_WAS_STILL_DRAWING:
		puts("DXGI_ERROR_WAS_STILL_DRAWING");
		break;
	case DXGI_ERROR_UNSUPPORTED:
		puts("DXGI_ERROR_UNSUPPORTED");
		break;
	case DXGI_ERROR_ACCESS_LOST:
		puts("DXGI_ERROR_ACCESS_LOST");
		break;
	case DXGI_ERROR_WAIT_TIMEOUT:
		puts("DXGI_ERROR_WAIT_TIMEOUT");
		break;
	case DXGI_ERROR_SESSION_DISCONNECTED:
		puts("DXGI_ERROR_SESSION_DISCONNECTED");
		break;
	case DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE:
		puts("DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE");
		break;
	case DXGI_ERROR_CANNOT_PROTECT_CONTENT:
		puts("DXGI_ERROR_CANNOT_PROTECT_CONTENT");
		break;
	case DXGI_ERROR_ACCESS_DENIED:
		puts("DXGI_ERROR_ACCESS_DENIED");
		break;
	case DXGI_ERROR_NAME_ALREADY_EXISTS:
		puts("DXGI_ERROR_NAME_ALREADY_EXISTS");
		break;
	case DXGI_ERROR_SDK_COMPONENT_MISSING:
		puts("DXGI_ERROR_SDK_COMPONENT_MISSING");
		break;
	default:
		break;
	}
}

inline void setResourceBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
{
	D3D12_RESOURCE_BARRIER descBarrier = {};

	descBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	descBarrier.Transition.pResource = resource;
	descBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	descBarrier.Transition.StateBefore = stateBefore;
	descBarrier.Transition.StateAfter = stateAfter;

	commandList->ResourceBarrier(1, &descBarrier);
}