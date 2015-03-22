#pragma once

class MeshInstance;
class FileAspect;
struct BVHBuildNode;
struct BVHPrimitiveInfo;
struct BVHBuildNode_SIMD;
struct BVHPrimitiveInfo_SIMD;
class TriangleMesh_SIMD;
struct AABB_SIMD;

namespace vx
{
	namespace gl
	{
		class Buffer;
	}

	struct MeshVertex;
	struct Transform;
}

#include "AABB.h"
#include <vector>
#include "Ray.h"
#include <vxLib/Allocator/StackAllocator.h>

class TriangleMesh
{
	std::vector<vx::float3> m_vertices;
	std::vector<U32> m_indices;

	bool intersect(const Ray &ray, const vx::float3 &a, const vx::float3 &b, const vx::float3 &c);
public:
	TriangleMesh() = default;
	TriangleMesh(std::vector<vx::float3> &&vertices, std::vector<U32> &&indices);

	bool intersect(const Ray &ray);

	const std::vector<vx::float3>& getVertices(){ return m_vertices; }
	const std::vector<U32>& getIndices(){ return m_indices; }
};

struct LinearBVHNode
{
	AABB bounds;
	union
	{
		U32 primOffset; // leaf
		U32 secondChildOffset; // interior
	};
	U8 primCount{0};
	U8 axis;
	U8 pad[2];
};

class BVH
{
	static const U32 s_maxNodes = 255u;

	LinearBVHNode* m_pNodes{ nullptr };
	TriangleMesh_SIMD* m_pPrimitves{nullptr};
	std::vector<TriangleMesh_SIMD> m_primitives0;
	std::vector<TriangleMesh> m_primitives{};
	vx::StackAllocator m_allocatorPrimData;
	U32 m_primCount{0};
	U32 m_maxPrimsInNode{ 255u };
	U32 m_maxPrimCount{ 0 };
	U32 m_maxTriangleCount{ 0 };

	BVHBuildNode* recursiveBuild(std::vector<BVHPrimitiveInfo> &buildData, U32 start, U32 end, U32* totalNodes, std::vector<const TriangleMesh*> &orderedPrims, U32 currentDepth, U32* maxDepth);
	BVHBuildNode_SIMD* recursiveBuild(const TriangleMesh_SIMD* primitives, BVHPrimitiveInfo_SIMD* buildData, U32 start, U32 end, U32* totalNodes, 
		const TriangleMesh_SIMD** orderedPrims, U32* orderedPrimsSize, vx::StackAllocator* pScratchAllocator);

	U32 flattenBVHTree(BVHBuildNode* node, U32* offset);
	U32 flattenBVHTree(BVHBuildNode_SIMD* node, U32* offset);

	bool intersectP(const AABB &bounds, const Ray &ray, const vx::float3 &invDir, const U32 dirIsNeg[3]);

	void createTriangleMesh(const vx::Transform &transform, const vx::MeshVertex* pMeshVertices, const U32* pMeshIndices, U32 indexCount,
		TriangleMesh_SIMD* pTriangleMesh, AABB_SIMD* pBounds);

	void createGpuBuffers(U32 maxnodeCount, U32 maxTriangleCount, vx::gl::Buffer* vertexBlock, vx::gl::Buffer* bvhBlock);
	void createGpuBuffers(const U32 nodeCount, vx::gl::Buffer* vertexBlock, vx::gl::Buffer* bvhBlock);
	void updateGpuBuffers(const U32 nodeCount, U32 primCount, vx::gl::Buffer* vertexBlock, vx::gl::Buffer* bvhBlock, vx::StackAllocator* pScratchAllocator);

public:
	BVH();
	~BVH();

	// preallocate static data
	void initialize(U32 maxPrimCount, U32 maxTriangles, vx::gl::Buffer* vertexBlock, vx::gl::Buffer* bvhBlock, vx::StackAllocator* pAllocator);

	void create(const MeshInstance* instances, U32 instanceCount, const FileAspect &fileAspect, vx::gl::Buffer* vertexBlock, vx::gl::Buffer* bvhBlock);
	void create(const MeshInstance* instances, U32 instanceCount, const FileAspect &fileAspect, vx::gl::Buffer* vertexBlock, vx::gl::Buffer* bvhBlock, vx::StackAllocator* pScratchAllocator);

	bool intersect(const Ray &ray);
};