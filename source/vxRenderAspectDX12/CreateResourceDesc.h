#pragma once

struct D3D12_RESOURCE_DESC;
struct D3D12_CLEAR_VALUE;
enum D3D12_RESOURCE_STATES;

#include <vxLib/types.h>

struct CreateResourceDesc
{
	u64 size;
	const D3D12_RESOURCE_DESC* resDesc;
	const D3D12_CLEAR_VALUE* clearValue;
	D3D12_RESOURCE_STATES state;
};