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

class ShaderManager;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct ID3D12Resource;

#include "RenderPass.h"
#include "Heap.h"
#include <vxLib/math/Vector.h>
#include "DescriptorHeap.h"

class GBufferRenderer : public RenderPass
{
	d3d::DescriptorHeap m_descriptorHeapRt;
	d3d::DescriptorHeap m_descriptorHeapDs;
	d3d::DescriptorHeap m_descriptorHeapSrv;
	d3d::Object<ID3D12Resource> m_diffuseSlice;
	d3d::Object<ID3D12Resource> m_normalSlice;
	d3d::Object<ID3D12Resource> m_velocitySlice;
	d3d::Object<ID3D12Resource> m_depthSlice;
	d3d::Heap m_gbufferHeap;

	bool loadShaders(ShaderManager* shaderManager);
	bool createRootSignature(ID3D12Device* device);
	bool createPipelineState(ID3D12Device* device, ShaderManager* shaderManager);
	bool createTextures(const vx::uint2 &resolution, d3d::Device* device);
	bool createDescriptorHeap(d3d::Device* device);

public:
	GBufferRenderer();
	~GBufferRenderer();

	bool initialize(const vx::uint2 &resolution, d3d::Device* device, ShaderManager* shaderManager);

	void submitCommands(ID3D12GraphicsCommandList* cmdList) override;

	void bindSrvBegin(ID3D12GraphicsCommandList* cmdList);
	void bindSrvEnd(ID3D12GraphicsCommandList* cmdList);
};