#pragma once

namespace vx
{
	namespace gl
	{
		class Buffer;
	}
}

class MeshInstance;
class FileAspect;

#include "StackAllocator.h"

// used for updating bvh stuff on gpu
class BVH_GPU
{
	vx::StackAllocator m_scratchAllocator;
	U32 m_maxPrimCount;
	U32 m_maxTriangles;

	void createGpuBuffers(U32 maxnodeCount, U32 maxTriangleCount, vx::gl::Buffer* vertexBlock, vx::gl::Buffer* bvhBlock);
	void updateGpuBuffers(const U32 nodeCount, U32 primCount, vx::gl::Buffer* vertexBlock, vx::gl::Buffer* bvhBlock, vx::StackAllocator* pScratchAllocator);

public:
	void initialize(U32 maxPrimCount, U32 maxTriangles, vx::gl::Buffer* vertexBlock, vx::gl::Buffer* bvhBlock, vx::StackAllocator* pAllocator);

	void update(const MeshInstance* instances, U32 instanceCount, const FileAspect &fileAspect, vx::gl::Buffer* vertexBlock, vx::gl::Buffer* bvhBlock, vx::StackAllocator* pScratchAllocator);
};