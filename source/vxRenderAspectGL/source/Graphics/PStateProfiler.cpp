#include <vxRenderAspect/Graphics/PStateProfiler.h>
#include <nvapi/nvapi.h>
#include <vxRenderAspect/Graphics/TextRenderer.h>

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

		NV_GPU_PERF_PSTATES_INFO info;
		info.version = NV_GPU_PERF_PSTATES_INFO_VER;
		r = NvAPI_GPU_GetPstatesInfoEx((NvPhysicalGpuHandle)m_gpuHandle, &info, 0);

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
		VX_ASSERT(size <= sizeof(buffer));
		std::string str;
		str.assign(buffer, buffer + size);

		textRenderer->pushEntry(std::move(str), position, vx::float3(0, 1, 0));
	}
}