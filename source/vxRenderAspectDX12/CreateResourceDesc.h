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

	static CreateResourceDesc createDesc(u64 size, const D3D12_RESOURCE_DESC* resDesc, const D3D12_CLEAR_VALUE* clearValue, D3D12_RESOURCE_STATES state)
	{
		CreateResourceDesc desc;
		desc.size = size;
		desc.resDesc = resDesc;
		desc.clearValue = clearValue;
		desc.state = state;

		return desc;
	}
};