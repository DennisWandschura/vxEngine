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

#include "PipelineState.h"

namespace PipelineStateCpp
{
	void writeShader(ID3D10Blob* shader, D3D12_SHADER_BYTECODE* byteCode)
	{
		if (shader)
		{
			byteCode->pShaderBytecode = reinterpret_cast<UINT8*>(shader->GetBufferPointer());;
			byteCode->BytecodeLength = shader->GetBufferSize();
		}
	}
}

namespace d3d
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineState::getDefaultDescription(const PipelineStateDescInput &inputDesc)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc;
		memset(&desc, 0, sizeof(desc));

		desc.InputLayout = inputDesc.inputLayout;
		desc.pRootSignature = inputDesc.rootSignature;

		PipelineStateCpp::writeShader(inputDesc.shaderDesc.vs, &desc.VS);
		PipelineStateCpp::writeShader(inputDesc.shaderDesc.gs, &desc.GS);
		PipelineStateCpp::writeShader(inputDesc.shaderDesc.ps, &desc.PS);

		desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		desc.RasterizerState.FrontCounterClockwise = 1;
		desc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		desc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		desc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		desc.RasterizerState.DepthClipEnable = TRUE;
		//desc.RasterizerState.MultisampleEnable = FALSE;
		//desc.RasterizerState.AntialiasedLineEnable = FALSE;
		//desc.RasterizerState.ForcedSampleCount = 0;
		desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		//desc.BlendState.AlphaToCoverageEnable = FALSE;
		//desc.BlendState.IndependentBlendEnable = FALSE;
		const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
		{
			FALSE,FALSE,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL,
		};
		for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
			desc.BlendState.RenderTarget[i] = defaultRenderTargetBlendDesc;

		desc.DepthStencilState.DepthEnable = inputDesc.depthEnabled;

		desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		//desc.DepthStencilState.StencilEnable = FALSE;
		desc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
		desc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
		const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
		{ D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
		desc.DepthStencilState.FrontFace = defaultStencilOp;
		desc.DepthStencilState.BackFace = defaultStencilOp;

		desc.SampleMask = UINT_MAX;
		desc.PrimitiveTopologyType = inputDesc.primitiveTopology;
		desc.SampleDesc.Count = 1;
		//desc.SampleDesc.Quality = 0;

		desc.NumRenderTargets = inputDesc.rtvCount;
		for (u32 i = 0; i < inputDesc.rtvCount; ++i)
		{
			desc.RTVFormats[i] = inputDesc.rtvFormats[i];
		}

		desc.DSVFormat = inputDesc.dsvFormat;

		return desc;
	}

	bool PipelineState::create(const D3D12_GRAPHICS_PIPELINE_STATE_DESC &desc, PipelineState* state, ID3D12Device* device)
	{
		auto hr = device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(state->m_state.getAddressOf()));

		return (hr == 0);
	}
}