#include "BVH.h"
#include "Primitive.h"
#include <algorithm>
#include "Ray.h"

struct LinearBVHNode
{
	AABB bounds;
	union
	{
		U32 primOffset; // leaf
		U32 secondChildOffset; // interior
	};
	U8 primCount{ 0 };
	U8 axis;
	U8 pad[2];
};

namespace
{
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

	struct BucketInfo
	{
		U32 count{ 0 };
		AABB bounds;
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

	BVHBuildNode* recursiveBuild(const std::vector<const Primitive*> &primitives, U32 maxPrimsInNode, std::vector<BVHPrimitiveInfo> &buildData, U32 start, U32 end, U32* totalNodes, std::vector<const Primitive*> &orderedPrims)
	{
		auto createLeaf = [](std::vector<const Primitive*> &orderedPrims, U32 start, U32 end, const std::vector<BVHPrimitiveInfo> &buildData,
			const std::vector<const Primitive*> &primitves, BVHBuildNode* node, U32 primCount, const AABB &bbox)
		{
			U32 firstPrimOffset = orderedPrims.size();
			for (U32 i = start; i < end; ++i)
			{
				U32 primNumber = buildData[i].primitiveNumber;
				orderedPrims.push_back(primitves[primNumber]);
			}
			node->initLeaf(firstPrimOffset, primCount, bbox);
		};

		++(*totalNodes);
		BVHBuildNode* node = new BVHBuildNode();

		AABB bbox;
		for (U32 i = start; i < end; ++i)
		{
			bbox = AABB::merge(bbox, buildData[i].bounds);
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
			createLeaf(orderedPrims, start, end, buildData, primitives, node, primCount, bbox);
		}
		else
		{
			AABB centroidBounds;
			for (U32 i = start; i < end; ++i)
			{
				centroidBounds = AABB::merge(centroidBounds, buildData[i].centroid);
			}
			U32 dim = centroidBounds.maximumExtend();

			U32 mid = (start + end) / 2;
			if (centroidBounds.max[dim] == centroidBounds.min[dim])
			{
				/*U32 firstPrimOffset = orderedPrims.size();
				for (U32 i = start; i < end; ++i)
				{
				U32 primNumber = buildData[i].primitiveNumber;
				orderedPrims.push_back(m_primitives[primNumber]);
				}
				node->initLeaf(firstPrimOffset, primCount, bbox);*/
				createLeaf(orderedPrims, start, end, buildData, primitives, node, primCount, bbox);

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
					buckets[b].bounds = AABB::merge(buckets[b].bounds, buildData[i].bounds);
				}

				// compute costs
				F32 cost[bucketCount - 1];
				for (U32 i = 0; i < bucketCount - 1; ++i)
				{
					AABB b0, b1;
					U32 count0 = 0, count1 = 0;
					for (U32 j = 0; j <= i; ++j)
					{
						b0 = AABB::merge(b0, buckets[j].bounds);
						count0 += buckets[j].count;
					}
					for (U32 j = i + 1; j < bucketCount; ++j)
					{
						b1 = AABB::merge(b1, buckets[j].bounds);
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
				if (primCount > maxPrimsInNode || minCost < primCount)
				{
					// split
					BVHPrimitiveInfo* pmid = std::partition(&buildData[start], &buildData[end - 1] + 1, CompareToBucket(minSplitCost, bucketCount, dim, centroidBounds));
					mid = pmid - &buildData[0];
				}
				else
				{
					// create leaf
					createLeaf(orderedPrims, start, end, buildData, primitives, node, primCount, bbox);
				}
			}

			node->initInterior(dim, recursiveBuild(primitives, maxPrimsInNode, buildData, start, mid, totalNodes, orderedPrims),
				recursiveBuild(primitives,maxPrimsInNode, buildData, mid, end, totalNodes, orderedPrims));
		}

		return node;
	}

	U32 flattenBVHTree(LinearBVHNode* pNodes, BVHBuildNode* node, U32* offset)
	{
		LinearBVHNode* linearNode = &pNodes[*offset];
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
			flattenBVHTree(pNodes, node->children[0], offset);
			linearNode->secondChildOffset = flattenBVHTree(pNodes, node->children[1], offset);
		}

		return myOffset;
	}
}

BVH::BVH()
	:m_pNodes(),
	m_primCount(0),
	m_maxPrimsInNode(255u),
	m_primitives()
{
}

BVH::~BVH()
{
}

void BVH::create(const Primitive* primitives, U32 primitiveCount)
{
	m_maxPrimsInNode = std::min(255u, primitiveCount);

	std::vector<BVHPrimitiveInfo> buildData;
	buildData.reserve(primitiveCount);
	m_primitives.reserve(primitiveCount);
	for (U32 i = 0; i < primitiveCount; ++i)
	{
		auto bounds = primitives[i].worldBound();

		buildData.push_back(BVHPrimitiveInfo(i, bounds));

		m_primitives.push_back(&primitives[i]);
	}

	U32 totalNodes = 0;
	std::vector<const Primitive*> orderedPrims;
	orderedPrims.reserve(primitiveCount);
	auto root = recursiveBuild(m_primitives, m_maxPrimsInNode, buildData, 0, primitiveCount, &totalNodes, orderedPrims);

	std::vector<const Primitive*> tmp;
	tmp.reserve(primitiveCount);
	for (auto &it : orderedPrims)
	{
		tmp.push_back(it);
	}
	m_primitives.swap(tmp);

	m_pNodes = std::make_unique<LinearBVHNode[]>(totalNodes);
	U32 offset = 0;
	flattenBVHTree(m_pNodes.get(), root, &offset);
}

bool BVH::intersect(const Ray &ray) const
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
					if (m_primitives[node->primOffset + i]->intersects(ray))
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

bool BVH::intersectP(const AABB &bounds, const Ray &ray, const vx::float3 &invDir, const U32 dirIsNeg[3]) const
{
	// Check for ray intersection against $x$ and $y$ slabs
	float tmin = (bounds[dirIsNeg[0]].x - ray.o.x) * invDir.x;
	float tmax = (bounds[1 - dirIsNeg[0]].x - ray.o.x) * invDir.x;
	float tymin = (bounds[dirIsNeg[1]].y - ray.o.y) * invDir.y;
	float tymax = (bounds[1 - dirIsNeg[1]].y - ray.o.y) * invDir.y;

	if ((tmin > tymax) || (tymin > tmax))
		return false;

	if (tymin > tmin) 
		tmin = tymin;
	if (tymax < tmax) 
		tmax = tymax;

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