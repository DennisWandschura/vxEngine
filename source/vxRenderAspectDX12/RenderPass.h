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

struct ID3D12GraphicsCommandList;
struct ID3D12RootSignature;
struct ID3D12PipelineState;
struct ID3D12Device;

#include "d3d.h"

namespace d3d
{
	class ShaderManager;
	class ResourceManager;
}

class RenderPass
{
protected:
	d3d::Object<ID3D12RootSignature> m_rootSignature;
	d3d::Object<ID3D12PipelineState> m_pipelineState;

	RenderPass() :m_rootSignature(), m_pipelineState() {}

public:
	virtual ~RenderPass() {}

	virtual bool initialize(d3d::ShaderManager* shaderManager, d3d::ResourceManager* resourceManager, ID3D12Device* device, void* p) = 0;
	virtual void shutdown() = 0;

	virtual void submitCommands(ID3D12GraphicsCommandList* cmdList) = 0;
};