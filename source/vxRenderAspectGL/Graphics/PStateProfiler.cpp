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

#include "PStateProfiler.h"
#include <nvapi/nvapi.h>
#include "TextRenderer.h"

namespace Graphics
{
	PStateProfiler::PStateProfiler()
		:m_gpuHandle(nullptr),
		m_position(0,0)
	{

	}

	PStateProfiler::~PStateProfiler()
	{

	}

	bool PStateProfiler::initialize(const vx::float2 &position)
	{
		auto r = NvAPI_Initialize();

		NvPhysicalGpuHandle gpuHandles[64];
		NvU32 gpuCount = 0;
		NvAPI_EnumPhysicalGPUs(gpuHandles, &gpuCount);
		m_gpuHandle = gpuHandles[0];

		NV_GPU_PERF_PSTATES20_INFO info;
		info.version = NV_GPU_PERF_PSTATES_INFO_VER;
		r = NvAPI_GPU_GetPstates20((NvPhysicalGpuHandle)m_gpuHandle, &info);

		m_position = position;

		return true;
	}

	void PStateProfiler::update(TextRenderer* textRenderer)
	{
		auto position = m_position;

		NV_GPU_PERF_PSTATE_ID currentPState{};
		NvAPI_GPU_GetCurrentPstate((NvPhysicalGpuHandle)m_gpuHandle, &currentPState);
		NV_GPU_DYNAMIC_PSTATES_INFO_EX info{};
		info.version = NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER;
		NvAPI_GPU_GetDynamicPstatesInfoEx((NvPhysicalGpuHandle)m_gpuHandle, &info);

		auto utilGPU = info.utilization[0].percentage;
		auto utilFB = info.utilization[1].percentage;
		auto utilVID = info.utilization[2].percentage;
		auto utilBUS = info.utilization[3].percentage;

		char buffer[48];
		auto size = sprintf(buffer, "PState: %u\nGpu: %lu\nFB: %lu\nVid: %lu\nBus: %lu", (u32)currentPState, utilGPU, utilFB, utilVID, utilBUS);
		/*VX_ASSERT(size <= sizeof(buffer));
		std::string str;
		str.assign(buffer, buffer + size);*/

		textRenderer->pushEntry(buffer, size, position, vx::float3(0, 1, 0));
	}
}