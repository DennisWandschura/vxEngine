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

struct ID3D12RootSignature;
struct ID3D12PipelineState;
struct ID3D12Device;
class UploadManager;
class GpuProfiler;

#include "d3d.h"
#include <vxLib/math/Vector.h>
#include "RenderSettings.h"
#include "PipelineState.h"
#include <vxEngineLib/Graphics/CommandQueue.h>
#include "RootSignature.h"

namespace d3d
{
	class ShaderManager;
	class ResourceManager;
}

class RenderPass
{
protected:
	static d3d::ShaderManager* s_shaderManager; 
	static d3d::ResourceManager* s_resourceManager;
	static UploadManager* s_uploadManager;
	static vx::uint2 s_resolution;
	static const RenderSettings* s_settings;
	static GpuProfiler* s_gpuProfiler;

	d3d::RootSignature m_rootSignature;
	d3d::PipelineState m_pipelineState;

	RenderPass();

	bool loadShaders(const wchar_t* const* name, u32 count);

public:
	virtual ~RenderPass();

	static void provideData(d3d::ShaderManager* shaderManager, d3d::ResourceManager* resourceManager, UploadManager* uploadManager, const RenderSettings* settings, GpuProfiler* gpuProfiler)
	{
		s_shaderManager = shaderManager;
		s_resourceManager = resourceManager;
		s_uploadManager = uploadManager;
		s_resolution = settings->m_resolution;
		s_settings = settings;
		s_gpuProfiler = gpuProfiler;
	}

	virtual void getRequiredMemory(u64* heapSizeBuffer, u32* bufferCount, u64* heapSizeTexture, u32* textureCount, u64* heapSizeRtDs, u32* rtDsCount, ID3D12Device* device) = 0;

	virtual bool createData(ID3D12Device* device) = 0;

	virtual bool initialize(ID3D12Device* device, void* p) = 0;
	virtual void shutdown() = 0;

	virtual void buildCommands() = 0;
	virtual void submitCommands(Graphics::CommandQueue* queue) = 0;
};