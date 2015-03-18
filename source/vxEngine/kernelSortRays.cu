#undef __INTEL_COMPILER
#define _VX_CUDA

#include <thrust/sort.h>
#include <Windows.h>
#include <cuda_gl_interop.h>
#include <thrust/device_ptr.h>

struct __builtin_align__(16) CompressedRay
{
	uint32_t mortonCode;
	uint32_t compressedRay[3];

	__device__ friend bool operator<(const CompressedRay &l, const CompressedRay &r)
	{
		return l.mortonCode < r.mortonCode;
	}
};

struct __builtin_align__(8) RayLink
{
	uint32_t mortonCode;
	uint32_t rayIndex;

	__device__ friend bool operator<(const RayLink &l, const RayLink &r)
	{
		return l.mortonCode < r.mortonCode;
	}
};

cudaGraphicsResource_t g_rayLinkBuffer{nullptr};
cudaGraphicsResource_t g_rayListBuffer{ nullptr };

void registerRayLinkBuffer(uint32_t rayLinkBufferId)
{
	cudaGraphicsGLRegisterBuffer(&g_rayLinkBuffer, rayLinkBufferId, cudaGraphicsRegisterFlagsNone);
}

void registerRayListBuffer(uint32_t rayListBufferId)
{
	cudaGraphicsGLRegisterBuffer(&g_rayListBuffer, rayListBufferId, cudaGraphicsRegisterFlagsNone);
}

void initializeCUDA(uint32_t rayLinkBufferId, uint32_t rayListBufferId)
{
	registerRayLinkBuffer(rayLinkBufferId);
	registerRayListBuffer(rayListBufferId);
}

void shutdownCUDA()
{
	cudaGraphicsUnregisterResource(g_rayLinkBuffer);
	cudaGraphicsUnregisterResource(g_rayListBuffer);

	g_rayLinkBuffer = nullptr;
	g_rayListBuffer = nullptr;
}

void cudaSortRayLinks(uint32_t rayLinkCount)
{
	cudaGraphicsMapResources(1, &g_rayListBuffer);

	size_t sizeInBytes = 0;
	void* rayLinkPtr = nullptr;
	cudaGraphicsResourceGetMappedPointer(&rayLinkPtr, &sizeInBytes, g_rayLinkBuffer);

	thrust::device_ptr<RayLink> devicePtr((RayLink*)rayLinkPtr);

	thrust::sort(devicePtr, devicePtr + rayLinkCount);

	cudaGraphicsUnmapResources(1, &g_rayLinkBuffer);
}

void cudaSortRayList(uint32_t rayCount)
{
	cudaGraphicsMapResources(1, &g_rayListBuffer);

	size_t sizeInBytes = 0;
	void* rayListPtr = nullptr;
	cudaGraphicsResourceGetMappedPointer(&rayListPtr, &sizeInBytes, g_rayListBuffer);

	thrust::device_ptr<CompressedRay> devicePtr((CompressedRay*)rayListPtr);

	thrust::sort(devicePtr, devicePtr + rayCount);

	cudaGraphicsUnmapResources(1, &g_rayListBuffer);
}