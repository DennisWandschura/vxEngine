#include "BVH.h"
#include "MeshInstance.h"
#include "FileAspect.h"
#include <vxLib/gl/Buffer.h>
#include "utility.h"
#include "UniformBlocks.h"
#include <vxLib/ScopeGuard.h>
#include "Triangle.h"

struct AABB_SIMD
{
	union
	{
		__m128 v[2];
		struct
		{
			__m128 min;
			__m128 max;
		};
	};


	AABB_SIMD()
	{
		v[0] = _mm_set1_ps(FLT_MAX);
		v[1] = _mm_set1_ps(-FLT_MAX);
	}

	AABB_SIMD Union(const AABB_SIMD &other) const
	{
		AABB_SIMD r;
		r.v[0] = _mm_min_ps(v[0], other.v[0]);
		r.v[1] = _mm_max_ps(v[1], other.v[1]);
		return r;
	}

	AABB_SIMD Union(const __m128 &p) const
	{
		AABB_SIMD r;
		r.v[0] = _mm_min_ps(v[0], p);
		r.v[1] = _mm_max_ps(v[1], p);
		return r;
	}

	U32 maximumExtend() const
	{
		auto d = _mm_sub_ps(max, min);

		U32 maxExtend = 2;
		if (d.f[0] > d.f[1] && d.f[0] > d.f[2])
		{
			maxExtend = 0;
		}
		else if (d.f[1] > d.f[2])
		{
			maxExtend = 1;
		}

		return maxExtend;
	}

	F32 surfaceArea() const
	{
		auto d = _mm_sub_ps(max, min);

		auto d0 = _mm_shuffle_ps(d, d, _MM_SHUFFLE(3, 1, 0, 0));
		auto d1 = _mm_shuffle_ps(d, d, _MM_SHUFFLE(3, 2, 2, 1));

		const int mask = (1 << 0) | (1 << 1) | (1 << 2) | (0 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (0 << 7);
		d = _mm_dp_ps(d0, d1, mask);
		d = _mm_mul_ss(d, vx::g_VXTwo);

		F32 result;
		_mm_store_ss(&result, d);

		return result;
	}
};

struct TriangleSIMD
{
	__m128 v[3];
};

class TriangleMesh_SIMD
{
	TriangleSIMD* m_triangles{ nullptr };
	U32 m_triangleCount{ 0 };

public:
	TriangleMesh_SIMD() = default;
	TriangleMesh_SIMD(TriangleSIMD* triangles, U32 triangleCount)
		:m_triangles(triangles), m_triangleCount(triangleCount){}

	const TriangleSIMD* getTriangles() const{ return m_triangles; }
	TriangleSIMD* getTriangles() { return m_triangles; }
	U32 getTriangleCount() const { return m_triangleCount; }
};

struct BVHBuildNode_SIMD
{
	AABB_SIMD bounds;
	BVHBuildNode_SIMD* children[2];
	U32 splitAxis;
	U32 firstPrimOffset;
	U32 primCount;

	BVHBuildNode_SIMD() :primCount(0){ children[0] = children[1] = nullptr; }

	void initLeaf(U32 first, U32 n, const AABB_SIMD &b)
	{
		firstPrimOffset = first;
		primCount = n;
		bounds = b;
	}

	void initInterior(U32 axis, BVHBuildNode_SIMD* c0, BVHBuildNode_SIMD* c1)
	{
		children[0] = c0;
		children[1] = c1;

		bounds.v[0] = _mm_min_ps(c0->bounds.v[0], c1->bounds.v[0]);
		bounds.v[1] = _mm_max_ps(c0->bounds.v[1], c1->bounds.v[1]);
		splitAxis = axis;
		primCount = 0;
	}
};

struct BVHBuildNode
{
	AABB bounds;
	BVHBuildNode* children[2];
	U32 splitAxis;
	U32 firstPrimOffset;
	U32 primCount;

	BVHBuildNode() :primCount(0){ children[0] = children[1] = nullptr; }

	void initLeaf(U32 first, U32 n, const AABB &b)
	{
		firstPrimOffset = first;
		primCount = n;
		bounds = b;
	}

	void initInterior(U32 axis, BVHBuildNode* c0, BVHBuildNode* c1)
	{
		children[0] = c0;
		children[1] = c1;
		bounds.max = vx::max(c0->bounds.max, c1->bounds.max);
		bounds.min = vx::min(c0->bounds.min, c1->bounds.min);
		splitAxis = axis;
		primCount = 0;
	}
};

struct BVHPrimitiveInfo_SIMD
{
	U32 primitiveNumber;
	__m128 centroid;
	AABB_SIMD bounds;

	BVHPrimitiveInfo_SIMD(U32 pn, const AABB_SIMD &b)
		:primitiveNumber(pn), bounds(b)
	{
		auto tmp = _mm_mul_ps(vx::g_VXOneHalf, b.v[0]);
		centroid = _mm_mul_ps(vx::g_VXOneHalf, b.v[1]);

		centroid = _mm_add_ps(tmp, centroid);
		//centroid = 0.5f * b.min + .5f * b.max;
	}
};

struct BVHPrimitiveInfo
{
	U32 primitiveNumber;
	vx::float3 centroid;
	AABB bounds;

	BVHPrimitiveInfo(U32 pn, const AABB &b)
		:primitiveNumber(pn), bounds(b)
	{
		centroid = 0.5f * b.min + .5f * b.max;
	}
};

struct ComparePoints
{
	U32 dim;

	ComparePoints(U32 d) :dim(d){}

	bool operator()(const BVHPrimitiveInfo &lhs, const BVHPrimitiveInfo &rhs) const
	{
		return lhs.centroid[dim] < rhs.centroid[dim];
	}
};

struct ComparePoints_SIMD
{
	U32 dim;

	ComparePoints_SIMD(U32 d) :dim(d){}

	bool operator()(const BVHPrimitiveInfo_SIMD &lhs, const BVHPrimitiveInfo_SIMD &rhs) const
	{
		return lhs.centroid.f[dim] < rhs.centroid.f[dim];
	}
};

struct BucketInfo
{
	U32 count{ 0 };
	AABB bounds;
};

struct BucketInfo_SIMD
{
	U32 count{ 0 };
	AABB_SIMD bounds;
};

struct CompareToBucket_SIMD
{
	U32 splitBucket, bucketCount, dim;
	const AABB_SIMD &centroidBounds;

	CompareToBucket_SIMD(U32 split, U32 count, U32 d, const AABB_SIMD &b)
		:splitBucket(split), bucketCount(count), dim(d), centroidBounds(b) {}

	bool operator()(const BVHPrimitiveInfo_SIMD &p) const
	{
		U32 b = bucketCount * ((p.centroid.f[dim] - centroidBounds.min.f[dim]) /
			(centroidBounds.max.f[dim] - centroidBounds.min.f[dim]));

		if (b == bucketCount)
			b = bucketCount - 1;

		return b <= splitBucket;
	}
};

struct CompareToBucket
{
	U32 splitBucket, bucketCount, dim;
	const AABB &centroidBounds;

	CompareToBucket(U32 split, U32 count, U32 d, const AABB &b)
		:splitBucket(split), bucketCount(count), dim(d), centroidBounds(b) {}

	bool operator()(const BVHPrimitiveInfo &p) const
	{
		U32 b = bucketCount * ((p.centroid[dim] - centroidBounds.min[dim]) /
			(centroidBounds.max[dim] - centroidBounds.min[dim]));

		if (b == bucketCount)
			b = bucketCount - 1;

		return b <= splitBucket;
	}
};

TriangleMesh::TriangleMesh(std::vector<vx::float3> &&vertices, std::vector<U32> &&indices)
	:m_vertices(std::move(vertices)), m_indices(std::move(indices))
{
}

bool TriangleMesh::intersect(const Ray &ray)
{
	bool hit = false;

	//auto triangleCount = m_indices.size() / 3;
	auto indexCount = m_indices.size();
	for (auto i = 0u; i < indexCount; i += 3)
	{
		U32 i0 = m_indices[i];
		U32 i1 = m_indices[i + 1];
		U32 i2 = m_indices[i + 2];

		if (intersect(ray, m_vertices[i0], m_vertices[i1], m_vertices[i2]))
		{
			hit = true;
			break;
		}
	}

	return hit;
}

bool TriangleMesh::intersect(const Ray &ray, const vx::float3 &a, const vx::float3 &b, const vx::float3 &c)
{
	auto e1 = b - a;
	auto e2 = c - a;
	auto s1 = vx::cross(ray.d, e2);
	auto divisor = vx::dot(s1, e1);
	if (divisor == 0.f)
		return false;

	auto invDivisor = 1.0f / divisor;

	auto s = ray.o - a;
	auto b1 = vx::dot(s, s1) * invDivisor;
	if (b1 < 0.0f || b1 > 1.0f)
		return false;

	auto s2 = vx::cross(s, e1);
	auto b2 = vx::dot(ray.d, s2) * invDivisor;
	if (b2 < 0.0f || b2 > 1.0f)
		return false;

	auto t = vx::dot(e2, s2) * invDivisor;
	if (t < ray.mint || t > ray.maxt)
		return false;

	return true;
}

namespace
{
	// applies transform and updates mesh aabb
	void applyTransformToTriangleMesh(const vx::Transform &transform, TriangleMesh_SIMD* pTriangleMesh, AABB_SIMD* pBounds)
	{
		auto qRotation = vx::loadFloat(&transform.m_rotation);
		qRotation = vx::QuaternionRotationRollPitchYawFromVector(qRotation);
		auto vTranslate = vx::loadFloat(&transform.m_translation);

		__m128 vScaling = { transform.m_scaling, transform.m_scaling, transform.m_scaling, 0.0 };

		auto pMeshTriangles = pTriangleMesh->getTriangles();
		auto triangleCount = pTriangleMesh->getTriangleCount();

		AABB_SIMD bounds;
		for (U32 i = 0; i < triangleCount; ++i)
		{
			auto &triangle = pMeshTriangles[i];

			// apply scaling
			triangle.v[0] = _mm_mul_ps(triangle.v[0], vScaling);
			triangle.v[1] = _mm_mul_ps(triangle.v[1], vScaling);
			triangle.v[2] = _mm_mul_ps(triangle.v[2], vScaling);

			// apply rotation
			triangle.v[0] = vx::Vector3Rotate(triangle.v[0], qRotation);
			triangle.v[1] = vx::Vector3Rotate(triangle.v[1], qRotation);
			triangle.v[2] = vx::Vector3Rotate(triangle.v[2], qRotation);

			// apply translation
			triangle.v[0] = _mm_add_ps(triangle.v[0], vTranslate);
			triangle.v[1] = _mm_add_ps(triangle.v[1], vTranslate);
			triangle.v[2] = _mm_add_ps(triangle.v[2], vTranslate);

			// update aabb

			bounds.v[0] = _mm_min_ps(bounds.v[0], triangle.v[0]);
			bounds.v[1] = _mm_max_ps(bounds.v[1], triangle.v[0]);

			bounds.v[0] = _mm_min_ps(bounds.v[0], triangle.v[1]);
			bounds.v[1] = _mm_max_ps(bounds.v[1], triangle.v[1]);

			bounds.v[0] = _mm_min_ps(bounds.v[0], triangle.v[2]);
			bounds.v[1] = _mm_max_ps(bounds.v[1], triangle.v[2]);
		}

		*pBounds = bounds;
	}
}

BVH::BVH()
	:m_primitives0()
{

}

BVH::~BVH()
{
	if (m_pNodes)
	{
		//delete[](m_pNodes);
		m_pNodes = nullptr;
	}
}

void BVH::initialize(U32 maxPrimCount, U32 maxTriangleCount, vx::gl::Buffer* vertexBlock, vx::gl::Buffer* bvhBlock, vx::StackAllocator* pAllocator)
{
	m_pNodes = (LinearBVHNode*)pAllocator->allocate(sizeof(LinearBVHNode) * s_maxNodes, 16);
	m_pPrimitves = (TriangleMesh_SIMD*)pAllocator->allocate(sizeof(TriangleMesh_SIMD) * maxPrimCount, 16);
	pAllocator->rangeConstruct<TriangleMesh_SIMD>(m_pPrimitves, m_pPrimitves + maxPrimCount);

	const auto triangleBytes = sizeof(TriangleA) * maxTriangleCount;
	m_allocatorPrimData = vx::StackAllocator(pAllocator->allocate(triangleBytes, 16), triangleBytes);

	m_primitives0.reserve(maxPrimCount);

	m_maxPrimCount = maxPrimCount;
	m_maxTriangleCount = maxTriangleCount;

	if (vertexBlock && bvhBlock)
	{
		createGpuBuffers(s_maxNodes, m_maxTriangleCount, vertexBlock, bvhBlock);
	}
}

void BVH::create(const MeshInstance* instances, U32 instanceCount, const FileAspect &fileAspect, vx::gl::Buffer* vertexBlock, vx::gl::Buffer* bvhBlock)
{
	m_maxPrimsInNode = std::min(255u, instanceCount);

	std::vector<BVHPrimitiveInfo> buildData;
	buildData.reserve(instanceCount);
	m_primitives.reserve(instanceCount);
	for (U32 i = 0; i < instanceCount; ++i)
	{
		auto &instance = instances[i];
		auto meshSid = instance.getMeshSid();
		auto transform = instance.getTransform();
		auto qRotation = vx::loadFloat(&transform.m_rotation);
		qRotation = vx::QuaternionRotationRollPitchYawFromVector(qRotation);
		auto vTranslate = vx::loadFloat(&transform.m_translation);

		auto meshIt = fileAspect.getMesh(meshSid);
		auto pVertices = meshIt->getVertices();
		auto vertexCount = meshIt->getVertexCount();
		auto pIndices = meshIt->getIndices();
		auto indexCount = meshIt->getIndexCount();

		std::vector<vx::float3> vertices;
		vertices.reserve(vertexCount);

		AABB bounds;
		for (U32 j = 0; j < vertexCount; ++j)
		{
			auto position = pVertices[j].position * transform.m_scaling;

			auto vTmp = vx::loadFloat(&position);
			vTmp = vx::Vector3Rotate(vTmp, qRotation);
			vTmp = _mm_add_ps(vTmp, vTranslate);
			vx::storeFloat(&position, vTmp);
			// vec3 position = quaternationRotation(transform.scaling * inputPosition.xyz, qRotation) + transform.translation.xyz;

			// world space vertex
			vertices.push_back(position);

			bounds.min = vx::min(bounds.min, position);
			bounds.max = vx::max(bounds.max, position);
		}

		std::vector<U32> indices;
		indices.reserve(indexCount);
		for (U32 j = 0; j < indexCount; ++j)
		{
			indices.push_back(pIndices[j]);
		}

		buildData.push_back(BVHPrimitiveInfo(i, bounds));

		m_primitives.push_back(TriangleMesh(std::move(vertices), std::move(indices)));
	}

	U32 totalNodes = 0;
	std::vector<const TriangleMesh*> orderedPrims;
	orderedPrims.reserve(instanceCount);
	U32 maxDepth = 0;
	auto root = recursiveBuild(buildData, 0, instanceCount, &totalNodes, orderedPrims, 0, &maxDepth);

	VX_ASSERT(totalNodes <= s_maxNodes);

	std::vector<TriangleMesh> tmp;
	tmp.reserve(instanceCount);
	for (auto &it : orderedPrims)
	{
		tmp.push_back(std::move(*it));
	}
	m_primitives.swap(tmp);

	//m_pNodes = new LinearBVHNode[totalNodes];
	U32 offset = 0;
	flattenBVHTree(root, &offset);

	createGpuBuffers(totalNodes, vertexBlock, bvhBlock);
}

void BVH::createTriangleMesh(const vx::Transform &transform, const vx::MeshVertex* pMeshVertices, const U32* pMeshIndices, U32 indexCount,
	TriangleMesh_SIMD* pTriangleMesh, AABB_SIMD* pBounds)
{

	auto triangleCount = indexCount / 3;
	TriangleSIMD* pTriangles = (TriangleSIMD*)m_allocatorPrimData.allocate(sizeof(TriangleSIMD) * triangleCount, 16);

	// first create triangles

	for (U32 j = 0, i = 0; j < indexCount; j += 3, ++i)
	{
		auto i0 = pMeshIndices[j];
		auto i1 = pMeshIndices[j + 1];
		auto i2 = pMeshIndices[j + 2];

		pTriangles[i].v[0] = vx::loadFloat(&pMeshVertices[i0].position);
		pTriangles[i].v[1] = vx::loadFloat(&pMeshVertices[i1].position);
		pTriangles[i].v[2] = vx::loadFloat(&pMeshVertices[i2].position);
	}

	//U32* pIndices = (U32*)m_allocatorPrimData.allocate(sizeof(U32) * indexCount, 4);
	//vx::memcpy(pIndices, pMeshIndices, indexCount);

	*pTriangleMesh = TriangleMesh_SIMD(pTriangles, triangleCount);

	applyTransformToTriangleMesh(transform, pTriangleMesh, pBounds);
}

void BVH::create(const MeshInstance* instances, U32 instanceCount, const FileAspect &fileAspect, vx::gl::Buffer* vertexBlock, vx::gl::Buffer* bvhBlock, vx::StackAllocator* pScratchAllocator)
{
	VX_ASSERT(instanceCount <= m_maxPrimCount);

	m_maxPrimsInNode = std::min(255u, instanceCount);

	auto marker = pScratchAllocator->getMarker();

	auto guard = vx::scopeGuard([&]()
	{
		pScratchAllocator->clear(marker);
	});

	m_allocatorPrimData.clear();

	//std::vector<BVHPrimitiveInfo_SIMD> buildData;
	//buildData.reserve(instanceCount);
	BVHPrimitiveInfo_SIMD* buildData = (BVHPrimitiveInfo_SIMD*)pScratchAllocator->allocate(sizeof(BVHPrimitiveInfo_SIMD) * instanceCount, 16);

	//std::vector<TriangleMesh_SIMD> primitives;
	//primitives.reserve(instanceCount);
	TriangleMesh_SIMD* primitives = (TriangleMesh_SIMD*)pScratchAllocator->allocate(sizeof(TriangleMesh_SIMD) * instanceCount, 16);
	for (U32 i = 0; i < instanceCount; ++i)
	{
		auto &instance = instances[i];
		auto meshSid = instance.getMeshSid();
		auto transform = instance.getTransform();

		auto meshIt = fileAspect.getMesh(meshSid);
		auto pMeshVertices = meshIt->getVertices();
	//	auto vertexCount = meshIt->getVertexCount();
		auto pMeshIndices = meshIt->getIndices();
		auto indexCount = meshIt->getIndexCount();

		AABB_SIMD bounds;
		TriangleMesh_SIMD triangleMesh;
		createTriangleMesh(transform, pMeshVertices, pMeshIndices, indexCount, &triangleMesh, &bounds);

		//buildData.push_back(BVHPrimitiveInfo_SIMD(i, bounds));
		buildData[i] = BVHPrimitiveInfo_SIMD(i, bounds);

		//primitives.push_back(TriangleMesh_SIMD(pVertices, pIndices, vertexCount, indexCount));
		//primitives[i] = TriangleMesh_SIMD(pVertices, pIndices, vertexCount, indexCount);
		primitives[i] = triangleMesh;
	}

	U32 totalNodes = 0;
	//std::vector<const TriangleMesh_SIMD*> orderedPrims;
	//orderedPrims.reserve(instanceCount);
	const TriangleMesh_SIMD** orderedPrims = (const TriangleMesh_SIMD**)pScratchAllocator->allocate(sizeof(const TriangleMesh_SIMD*) * instanceCount, 16);

	U32 orderedPrimsSize = 0;
	auto root = recursiveBuild(primitives, buildData, 0, instanceCount, &totalNodes, orderedPrims, &orderedPrimsSize, pScratchAllocator);

	VX_ASSERT(totalNodes <= 255u);

	//std::vector<TriangleMesh_SIMD> tmp;
	//tmp.reserve(instanceCount);
	TriangleMesh_SIMD* tmp = (TriangleMesh_SIMD*)pScratchAllocator->allocate(sizeof(TriangleMesh_SIMD) * instanceCount, 16);

	m_primitives0.clear();
	//for (auto &it : orderedPrims)
	for (U32 i = 0; i < instanceCount; ++i)
	{
		//	tmp.push_back(std::move(*it));
		//tmp.push_back(std::move(*orderedPrims[i]));
		tmp[i] = std::move(*orderedPrims[i]);
		m_primitives0.push_back(tmp[i]);
	}
	vx::memcpy(m_pPrimitves, tmp, instanceCount);
	m_primCount = instanceCount;
	//	primitives.swap(tmp);

	//m_pNodes = new LinearBVHNode[totalNodes];
	U32 offset = 0;
	flattenBVHTree(root, &offset);

	guard.dismiss();
	pScratchAllocator->clear(marker);

	updateGpuBuffers(totalNodes, instanceCount, vertexBlock, bvhBlock, pScratchAllocator);
}

BVHBuildNode* BVH::recursiveBuild(std::vector<BVHPrimitiveInfo> &buildData, U32 start, U32 end, U32* totalNodes, std::vector<const TriangleMesh*> &orderedPrims, U32 currentDepth, U32* maxDepth)
{
	auto createLeaf = [](std::vector<const TriangleMesh*> &orderedPrims, U32 start, U32 end, const std::vector<BVHPrimitiveInfo> &buildData,
		const std::vector<TriangleMesh> &primitves, BVHBuildNode* node, U32 primCount, const AABB &bbox)
	{
		U32 firstPrimOffset = orderedPrims.size();
		for (U32 i = start; i < end; ++i)
		{
			U32 primNumber = buildData[i].primitiveNumber;
			orderedPrims.push_back(&primitves[primNumber]);
		}
		node->initLeaf(firstPrimOffset, primCount, bbox);
	};

	*maxDepth = std::max(*maxDepth, currentDepth);

	++(*totalNodes);
	BVHBuildNode* node = new BVHBuildNode();

	AABB bbox;
	for (U32 i = start; i < end; ++i)
	{
		bbox = bbox.Union(buildData[i].bounds);
	}

	U32 primCount = end - start;
	if (primCount == 1)
	{
		/*U32 firstPrimOffset = orderedPrims.size();
		for (U32 i = start; i < end; ++i)
		{
		U32 primNumber = buildData[i].primitiveNumber;
		orderedPrims.push_back(m_primitives[primNumber]);
		}
		node->initLeaf(firstPrimOffset, primCount, bbox);*/
		createLeaf(orderedPrims, start, end, buildData, m_primitives, node, primCount, bbox);
	}
	else
	{
		AABB centroidBounds;
		for (U32 i = start; i < end; ++i)
		{
			centroidBounds = centroidBounds.Union(buildData[i].centroid);
		}
		U32 dim = centroidBounds.maximumExtend();

		//U32 mid = (start + end) / 2;
		U32 mid = (start + end) >> 1;
		if (centroidBounds.max[dim] == centroidBounds.min[dim])
		{
			/*U32 firstPrimOffset = orderedPrims.size();
			for (U32 i = start; i < end; ++i)
			{
			U32 primNumber = buildData[i].primitiveNumber;
			orderedPrims.push_back(m_primitives[primNumber]);
			}
			node->initLeaf(firstPrimOffset, primCount, bbox);*/
			createLeaf(orderedPrims, start, end, buildData, m_primitives, node, primCount, bbox);

			return node;
		}

		// partition primitives based on SAH
		if (primCount <= 4)
		{
			mid = (start + end) >> 1;
			std::nth_element(&buildData[start], &buildData[mid], &buildData[end - 1] + 1, ComparePoints(dim));
		}
		else
		{
			// allocate BucketInfo
			const U32 bucketCount = 12;
			BucketInfo buckets[bucketCount];

			// initialize BucketInfo
			for (U32 i = start; i < end; ++i)
			{
				U32 b = bucketCount * ((buildData[i].centroid[dim] - centroidBounds.min[dim]) /
					(centroidBounds.max[dim] - centroidBounds.min[dim]));

				if (b == bucketCount)b = bucketCount - 1;
				++buckets[b].count;
				buckets[b].bounds = buckets[b].bounds.Union(buildData[i].bounds);
			}

			// compute costs
			F32 cost[bucketCount - 1];
			for (U32 i = 0; i < bucketCount - 1; ++i)
			{
				AABB b0, b1;
				U32 count0 = 0, count1 = 0;
				for (U32 j = 0; j <= i; ++j)
				{
					b0 = b0.Union(buckets[j].bounds);
					count0 += buckets[j].count;
				}
				for (U32 j = i + 1; j < bucketCount; ++j)
				{
					b1 = b1.Union(buckets[j].bounds);
					count1 += buckets[j].count;
				}
				cost[i] = .125f + (count0 * b0.surfaceArea() + count1 * b1.surfaceArea()) / bbox.surfaceArea();
			}

			// find bucket to split
			F32 minCost = cost[0];
			U32 minSplitCost = 0;
			for (U32 i = 1; i < bucketCount - 1; ++i)
			{
				if (cost[i] < minCost)
				{
					minCost = cost[i];
					minSplitCost = i;
				}
			}

			// create leaf or split
			if (primCount > m_maxPrimsInNode || minCost < primCount)
			{
				// split
				BVHPrimitiveInfo* pmid = std::partition(&buildData[start], &buildData[end - 1] + 1, CompareToBucket(minSplitCost, bucketCount, dim, centroidBounds));
				mid = pmid - &buildData[0];
			}
			else
			{
				// create leaf
				createLeaf(orderedPrims, start, end, buildData, m_primitives, node, primCount, bbox);
			}
		}

		node->initInterior(dim, recursiveBuild(buildData, start, mid, totalNodes, orderedPrims, currentDepth + 1, maxDepth),
			recursiveBuild(buildData, mid, end, totalNodes, orderedPrims, currentDepth + 1, maxDepth));
	}

	return node;
}

BVHBuildNode_SIMD* BVH::recursiveBuild(const TriangleMesh_SIMD* primitives, BVHPrimitiveInfo_SIMD* buildData, U32 start, U32 end, U32* totalNodes,
	const TriangleMesh_SIMD** orderedPrims, U32* orderedPrimsSize, vx::StackAllocator* pScratchAllocator)
{
	auto createLeaf = [](const TriangleMesh_SIMD** orderedPrims, U32 start, U32 end, const BVHPrimitiveInfo_SIMD* buildData,
		const TriangleMesh_SIMD* primitves, BVHBuildNode_SIMD* node, U32 primCount, const AABB_SIMD &bbox, U32* orderedPrimsSize)
	{
		//U32 firstPrimOffset = orderedPrims.size();
		U32 firstPrimOffset = *orderedPrimsSize;

		for (U32 i = start; i < end; ++i)
		{
			U32 primNumber = buildData[i].primitiveNumber;

			//orderedPrims.push_back(&primitves[primNumber]);

			orderedPrims[(*orderedPrimsSize)] = &primitves[primNumber];
			++(*orderedPrimsSize);
		}
		node->initLeaf(firstPrimOffset, primCount, bbox);
	};

	++(*totalNodes);
	BVHBuildNode_SIMD* node = (BVHBuildNode_SIMD*)pScratchAllocator->allocate(sizeof(BVHBuildNode_SIMD), 16);
	pScratchAllocator->construct<BVHBuildNode_SIMD>(node);

	AABB_SIMD bbox;
	for (U32 i = start; i < end; ++i)
	{
		bbox = bbox.Union(buildData[i].bounds);
	}

	U32 primCount = end - start;
	if (primCount == 1)
	{
		/*U32 firstPrimOffset = orderedPrims.size();
		for (U32 i = start; i < end; ++i)
		{
		U32 primNumber = buildData[i].primitiveNumber;
		orderedPrims.push_back(m_primitives[primNumber]);
		}
		node->initLeaf(firstPrimOffset, primCount, bbox);*/
		createLeaf(orderedPrims, start, end, buildData, primitives, node, primCount, bbox, orderedPrimsSize);
	}
	else
	{
		AABB_SIMD centroidBounds;
		for (U32 i = start; i < end; ++i)
		{
			centroidBounds = centroidBounds.Union(buildData[i].centroid);
		}
		U32 dim = centroidBounds.maximumExtend();

		//U32 mid = (start + end) / 2;
		U32 mid = (start + end) >> 1;
		if (centroidBounds.max.f[dim] == centroidBounds.min.f[dim])
		{
			/*U32 firstPrimOffset = orderedPrims.size();
			for (U32 i = start; i < end; ++i)
			{
			U32 primNumber = buildData[i].primitiveNumber;
			orderedPrims.push_back(m_primitives[primNumber]);
			}
			node->initLeaf(firstPrimOffset, primCount, bbox);*/
			createLeaf(orderedPrims, start, end, buildData, primitives, node, primCount, bbox, orderedPrimsSize);

			return node;
		}

		// partition primitives based on SAH
		if (primCount <= 4)
		{
			mid = (start + end) >> 1;
			std::nth_element(&buildData[start], &buildData[mid], &buildData[end - 1] + 1, ComparePoints_SIMD(dim));
		}
		else
		{
			// allocate BucketInfo
			const U32 bucketCount = 12;
			BucketInfo_SIMD buckets[bucketCount];

			// initialize BucketInfo
			for (U32 i = start; i < end; ++i)
			{
				U32 b = bucketCount * ((buildData[i].centroid.f[dim] - centroidBounds.min.f[dim]) /
					(centroidBounds.max.f[dim] - centroidBounds.min.f[dim]));

				if (b == bucketCount)b = bucketCount - 1;
				++buckets[b].count;
				buckets[b].bounds = buckets[b].bounds.Union(buildData[i].bounds);
			}

			F32 bbox_invSurfaceArea = 1.0f / bbox.surfaceArea();
			// compute costs
			F32 cost[bucketCount - 1];
			for (U32 i = 0; i < bucketCount - 1; ++i)
			{
				AABB_SIMD b0, b1;
				U32 count0 = 0, count1 = 0;
				for (U32 j = 0; j <= i; ++j)
				{
					b0 = b0.Union(buckets[j].bounds);
					count0 += buckets[j].count;
				}
				for (U32 j = i + 1; j < bucketCount; ++j)
				{
					b1 = b1.Union(buckets[j].bounds);
					count1 += buckets[j].count;
				}

				cost[i] = .125f +
					(count0 * b0.surfaceArea() + count1 * b1.surfaceArea())
					* bbox_invSurfaceArea;
			}

			// find bucket to split
			F32 minCost = cost[0];
			U32 minSplitCost = 0;
			for (U32 i = 1; i < bucketCount - 1; ++i)
			{
				if (cost[i] < minCost)
				{
					minCost = cost[i];
					minSplitCost = i;
				}
			}

			// create leaf or split
			if (primCount > m_maxPrimsInNode || minCost < primCount)
			{
				// split
				BVHPrimitiveInfo_SIMD* pmid = std::partition(&buildData[start], &buildData[end - 1] + 1, CompareToBucket_SIMD(minSplitCost, bucketCount, dim, centroidBounds));
				mid = pmid - &buildData[0];
			}
			else
			{
				// create leaf
				createLeaf(orderedPrims, start, end, buildData, primitives, node, primCount, bbox, orderedPrimsSize);
			}
		}

		node->initInterior(dim, recursiveBuild(primitives, buildData, start, mid, totalNodes, orderedPrims, orderedPrimsSize, pScratchAllocator),
			recursiveBuild(primitives, buildData, mid, end, totalNodes, orderedPrims, orderedPrimsSize, pScratchAllocator));
	}

	return node;
}

U32 BVH::flattenBVHTree(BVHBuildNode* node, U32* offset)
{
	LinearBVHNode* linearNode = &m_pNodes[*offset];
	linearNode->bounds = node->bounds;
	U32 myOffset = (*offset)++;
	if (node->primCount > 0)
	{
		linearNode->primOffset = node->firstPrimOffset;
		linearNode->primCount = node->primCount;
	}
	else
	{
		linearNode->axis = node->splitAxis;
		linearNode->primCount = 0;
		flattenBVHTree(node->children[0], offset);
		linearNode->secondChildOffset = flattenBVHTree(node->children[1], offset);
	}

	return myOffset;
}

U32 BVH::flattenBVHTree(BVHBuildNode_SIMD* node, U32* offset)
{
	LinearBVHNode* linearNode = &m_pNodes[*offset];

	vx::storeFloat(&linearNode->bounds.min, node->bounds.min);
	vx::storeFloat(&linearNode->bounds.max, node->bounds.max);

	U32 myOffset = (*offset)++;
	if (node->primCount > 0)
	{
		linearNode->primOffset = node->firstPrimOffset;
		linearNode->primCount = node->primCount;
	}
	else
	{
		linearNode->axis = node->splitAxis;
		linearNode->primCount = 0;
		flattenBVHTree(node->children[0], offset);
		linearNode->secondChildOffset = flattenBVHTree(node->children[1], offset);
	}

	return myOffset;
}

bool BVH::intersect(const Ray &ray)
{
	if (m_pNodes == nullptr)
		return false;

	bool hit = false;
	//vx::float3 origin = ray(ray.mint);
	vx::float3 invDir = 1.0f / ray.d;
	U32 dirIsNeg[3] = { invDir.x < 0, invDir.y < 0, invDir.z < 0 };

	U32 todoOffset = 0, nodeNum = 0;
	U32 todo[64];
	while (true)
	{
		auto node = &m_pNodes[nodeNum];

		if (intersectP(node->bounds, ray, invDir, dirIsNeg))
		{
			if (node->primCount > 0)
			{
				// intersect ray against primitives
				for (U32 i = 0; i < node->primCount; ++i)
				{
					if (m_primitives[node->primOffset + i].intersect(ray))
						hit = true;
				}

				if (todoOffset == 0) break;
				nodeNum = todo[--todoOffset];
			}
			else
			{
				// put node on stack
				if (dirIsNeg[node->axis])
				{
					todo[todoOffset++] = nodeNum + 1;
					nodeNum = node->secondChildOffset;
				}
				else
				{
					todo[todoOffset++] = node->secondChildOffset;
					nodeNum = nodeNum + 1;
				}
			}
		}
		else
		{
			if (todoOffset == 0) break;
			nodeNum = todo[--todoOffset];
		}
	}

	return hit;
}

bool BVH::intersectP(const AABB &bounds, const Ray &ray, const vx::float3 &invDir, const U32 dirIsNeg[3])
{
	// Check for ray intersection against $x$ and $y$ slabs
	float tmin = (bounds[dirIsNeg[0]].x - ray.o.x) * invDir.x;
	float tmax = (bounds[1 - dirIsNeg[0]].x - ray.o.x) * invDir.x;
	float tymin = (bounds[dirIsNeg[1]].y - ray.o.y) * invDir.y;
	float tymax = (bounds[1 - dirIsNeg[1]].y - ray.o.y) * invDir.y;

	if ((tmin > tymax) || (tymin > tmax))
		return false;

	if (tymin > tmin) tmin = tymin;
	if (tymax < tmax) tmax = tymax;

	// Check for ray intersection against $z$ slab
	float tzmin = (bounds[dirIsNeg[2]].z - ray.o.z) * invDir.z;
	float tzmax = (bounds[1 - dirIsNeg[2]].z - ray.o.z) * invDir.z;

	if ((tmin > tzmax) || (tzmin > tmax))
		return false;

	if (tzmin > tmin)
		tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;

	return (tmin < ray.maxt) && (tmax > ray.mint);
}

void BVH::createGpuBuffers(U32 maxnodeCount, U32 maxTriangleCount, vx::gl::Buffer* vertexBlock, vx::gl::Buffer* bvhBlock)
{
	{
		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Shader_Storage_Buffer;
		desc.immutable = 1;
		desc.flags = vx::gl::BufferStorageFlags::Write;
		desc.size = sizeof(TriangleA) * maxTriangleCount;
		vertexBlock->create(desc);
	}

	{
		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Shader_Storage_Buffer;
		desc.immutable = 1;
		desc.flags = vx::gl::BufferStorageFlags::Write;
		desc.size = sizeof(BVHNodeGpu) * maxnodeCount;
		bvhBlock->create(desc);
	}
}

void BVH::createGpuBuffers(const U32 nodeCount, vx::gl::Buffer* vertexBlock, vx::gl::Buffer* bvhBlock)
{
	U32 iCount = 0;
	for (auto &it : m_primitives)
	{
		iCount += it.getIndices().size();
	}

	struct Triangle
	{
		vx::float3 v[3];
	};

	std::vector<Triangle> triangles;
	triangles.reserve(iCount / 3);

	// triangle offset, triangle count
	std::vector<std::pair<U32, U32>> prims;
	prims.reserve(m_primitives.size());

	U32 triangleOffset = 0;
	for (auto &it : m_primitives)
	{
		auto &mv = it.getVertices();
		auto &mi = it.getIndices();
		auto indexCount = mi.size();

		U32 triangleCount = indexCount / 3;
		for (U32 i = 0; i < indexCount; i += 3)
		{
			Triangle tri;
			tri.v[0] = mv[mi[i]];
			tri.v[1] = mv[mi[i + 1]];
			tri.v[2] = mv[mi[i + 2]];

			triangles.push_back(tri);
		}

		prims.push_back(std::make_pair(triangleOffset, triangleCount));
		triangleOffset += (triangleCount);
	}

	{
		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Shader_Storage_Buffer;
#ifdef _VX_GL_45
		desc.immutable = 1;
		desc.flags = vx::gl::BufferStorageFlags::Write;
#else
		desc.usage = vx::gl::BufferDataUsage::Dynamic_Draw;
#endif
		desc.size = sizeof(Triangle) * triangles.size();
		desc.pData = triangles.data();
		vertexBlock->create(desc);

		//auto p = vertexBlock->map(vx::gl::Map::Write_Only);
		//vx::memcpy(p, triangles.data(), triangles.size());
		//vertexBlock->unmap();
	}

	auto nodes = std::make_unique<BVHNodeGpu[]>(nodeCount);
	for (U32 i = 0; i < nodeCount; ++i)
	{
		auto &node = m_pNodes[i];

		nodes[i].bmin = node.bounds.min;

		U32 triangleCount = 0;
		if (node.primCount == 0)
		{
			nodes[i].secondChildOffset = node.secondChildOffset;
		}
		else
		{
			// offset into 
			nodes[i].triangleOffset = prims[node.primOffset].first;

			for (U32 j = 0; j < node.primCount; ++j)
			{
				triangleCount += prims[node.primOffset + j].second;
			}
		}

		nodes[i].bmax = node.bounds.max;
		nodes[i].triangleCount = triangleCount;
		VX_ASSERT(nodes[i].triangleCount == triangleCount);
		nodes[i].axis = node.axis;
		nodes[i].pad = 0;
	}

	{
		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Shader_Storage_Buffer;
		desc.immutable = 1;
		desc.flags = vx::gl::BufferStorageFlags::Write;
		desc.size = sizeof(BVHNodeGpu) * nodeCount;
		desc.pData = nodes.get();
		bvhBlock->create(desc);
	}

	//cudaUpdateBVH((const cu::BVHNode*)nodes.get(), nodeCount);
	//	cudaUpdateMeshTriangles((const cu::Triangle*)triangles.data(), triangles.size());

	//openCL->updateBvhBuffers(nodes.get(), nodeCount, vertices.data(), iCount);
}

void BVH::updateGpuBuffers(const U32 nodeCount, U32 primCount, vx::gl::Buffer* vertexBlock, vx::gl::Buffer* bvhBlock, vx::StackAllocator* pScratchAllocator)
{
	auto marker = pScratchAllocator->getMarker();
	SCOPE_EXIT
	{
		pScratchAllocator->clear(marker);
	};

	U32 iCount = 0;
	for (auto &it : m_primitives)
	{
		iCount += it.getIndices().size();
	}

	auto triCount = iCount / 3;
	//std::vector<Triangle> triangles;
	//triangles.reserve(iCount / 3);
	TriangleA* triangles = (TriangleA*)pScratchAllocator->allocate(sizeof(TriangleA) * triCount, 8);
	U32 trianglesSize = 0;

	// triangle offset, triangle count
	//std::vector<std::pair<U32, U32>> prims;
	//prims.reserve(m_primitives.size());
	std::pair<U32, U32>* prims = (std::pair<U32, U32>*)pScratchAllocator->allocate(sizeof(std::pair<U32, U32>) * primCount, 4);
	//pScratchAllocator->rangeConstruct<std::pair<U32, U32>>(prims, );
	U32 primsSize = 0;

	U32 triangleOffset = 0;
	//for (auto &it : m_primitives)
	for (U32 k = 0; k < primCount; ++k)
	{
		const auto pTriangles = m_pPrimitves[k].getTriangles();
		auto triangleCount = m_pPrimitves[k].getTriangleCount();

		for (U32 i = 0; i < triangleCount; ++i)
		{
			TriangleA tri;
			vx::storeFloat(&tri.v[0], pTriangles[i].v[0]);
			vx::storeFloat(&tri.v[1], pTriangles[i].v[1]);
			vx::storeFloat(&tri.v[2], pTriangles[i].v[2]);

			/*tri.v[0] = mv[mi[i]];
			tri.v[1] = mv[mi[i + 1]];
			tri.v[2] = mv[mi[i + 2]];*/

			//triangles.push_back(tri);
			triangles[trianglesSize] = tri;
			++trianglesSize;
		}

		//prims.push_back(std::make_pair(triangleOffset, triangleCount));
		prims[primsSize] = std::make_pair(triangleOffset, triangleCount);
		++primsSize;
		triangleOffset += (triangleCount);
	}

	{
		auto p = vertexBlock->map<TriangleA>(vx::gl::Map::Write_Only);
		vx::memcpy(p, triangles, trianglesSize);
		vertexBlock->unmap();
	}

	//auto nodes = std::make_unique<BVHNodeGpu[]>(nodeCount);
	auto nodes = (BVHNodeGpu*)pScratchAllocator->allocate(sizeof(BVHNodeGpu) * nodeCount, 16);
	for (U32 i = 0; i < nodeCount; ++i)
	{
		auto &node = m_pNodes[i];

		nodes[i].bmin = node.bounds.min;

		U32 triangleCount = 0;
		if (node.primCount == 0)
		{
			nodes[i].secondChildOffset = node.secondChildOffset;
		}
		else
		{
			// offset into 
			nodes[i].triangleOffset = prims[node.primOffset].first;

			for (U32 j = 0; j < node.primCount; ++j)
			{
				triangleCount += prims[node.primOffset + j].second;
			}
		}

		nodes[i].bmax = node.bounds.max;
		nodes[i].triangleCount = triangleCount;
		VX_ASSERT(nodes[i].triangleCount == triangleCount);
		nodes[i].axis = node.axis;
		nodes[i].pad = 0;
	}

	{
		auto p = bvhBlock->map<BVHNodeGpu>(vx::gl::Map::Write_Only);
		vx::memcpy(p, nodes, nodeCount);
		bvhBlock->unmap();
	}
}