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

#include "d3dObject.h"
#include <d3d12.h>

namespace d3d
{
	struct PipelineShaderDesc
	{
		ID3D10Blob* vs;
		ID3D10Blob* ps;
		ID3D10Blob* gs;

		PipelineShaderDesc() :vs(nullptr), ps(nullptr), gs(nullptr) {}
	};

	struct PipelineStateDescInput
	{
		ID3D12RootSignature* rootSignature;
		u8 depthEnabled; 
		PipelineShaderDesc shaderDesc;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopology;
		D3D12_INPUT_LAYOUT_DESC inputLayout;
		const DXGI_FORMAT* rtvFormats; 
		u32 rtvCount;
		DXGI_FORMAT dsvFormat;

		PipelineStateDescInput()
			:rootSignature(nullptr),
			depthEnabled(1),
			shaderDesc(),
			primitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE),
			inputLayout(),
			rtvFormats(nullptr),
			rtvCount(0),
			dsvFormat(DXGI_FORMAT_UNKNOWN)
		{
			inputLayout.pInputElementDescs = nullptr;
			inputLayout.NumElements = 0;
		}
	};

	class PipelineState
	{
		Object<ID3D12PipelineState> m_state;

	public:
		PipelineState() {}
		~PipelineState() {}

		void destroy() { m_state.destroy(); }

		ID3D12PipelineState* get() { return m_state.get(); }

		static D3D12_GRAPHICS_PIPELINE_STATE_DESC getDefaultDescription(const PipelineStateDescInput &desc);

		static bool create(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc, PipelineState* state, ID3D12Device* device);
	};
}