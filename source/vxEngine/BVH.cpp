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
#include "BVH.h"
#include "Primitive.h"
#include <algorithm>
#include <vxEngineLib/Ray.h>

struct LinearBVHNode
{
	AABB bounds;
	union
	{
		u32 primOffset; // leaf
		u32 secondChildOffset; // interior
	};
	u8 primCount{ 0 };
	u8 axis;
	u8 pad[2];
};

namespace
{
	struct BVHBuildNode
	{
		AABB bounds;
		BVHBuildNode* children[2];
		u32 splitAxis;
		u32 firstPrimOffset;
		u32 primCount;

		BVHBuildNode() :primCount(0){ children[0] = children[1] = nullptr; }

		void initLeaf(u32 first, u32 n, const AABB &b)
		{
			firstPrimOffset = first;
			primCount = n;
			bounds = b;
		}

		void initInterior(u32 axis, BVHBuildNode* c0, BVHBuildNode* c1)
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
		u32 primitiveNumber;
		vx::float3 centroid;
		AABB bounds;

		BVHPrimitiveInfo(u32 pn, const AABB &b)
			:primitiveNumber(pn), bounds(b)
		{
			centroid = 0.5f * b.min + .5f * b.max;
		}
	};

	struct ComparePoints
	{
		u32 dim;

		ComparePoints(u32 d) :dim(d){}

		bool operator()(const BVHPrimitiveInfo &lhs, const BVHPrimitiveInfo &rhs) const
		{
			return lhs.centroid[dim] < rhs.centroid[dim];
		}
	};

	struct BucketInfo
	{
		u32 count{ 0 };
		AABB bounds;
	};

	struct CompareToBucket
	{
		u32 splitBucket, bucketCount, dim;
		const AABB &centroidBounds;

		CompareToBucket(u32 split, u32 count, u32 d, const AABB &b)
			:splitBucket(split), bucketCount(count), dim(d), centroidBounds(b) {}

		bool operator()(const BVHPrimitiveInfo &p) const
		{
			u32 b = bucketCount * ((p.centroid[dim] - centroidBounds.min[dim]) /
				(centroidBounds.max[dim] - centroidBounds.min[dim]));

			if (b == bucketCount)
				b = bucketCount - 1;

			return b <= splitBucket;
		}
	};

	BVHBuildNode* recursiveBuild(const std::vector<const Primitive*> &primitives, u32 maxPrimsInNode, std::vector<BVHPrimitiveInfo> &buildData, u32 start, u32 end, u32* totalNodes, std::vector<const Primitive*> &orderedPrims)
	{
		auto createLeaf = [](std::vector<const Primitive*> &orderedPrims, u32 start, u32 end, const std::vector<BVHPrimitiveInfo> &buildData,
			const std::vector<const Primitive*> &primitves, BVHBuildNode* node, u32 primCount, const AABB &bbox)
		{
			u32 firstPrimOffset = orderedPrims.size();
			for (u32 i = start; i < end; ++i)
			{
				u32 primNumber = buildData[i].primitiveNumber;
				orderedPrims.push_back(primitves[primNumber]);
			}
			node->initLeaf(firstPrimOffset, primCount, bbox);
		};

		++(*totalNodes);
		BVHBuildNode* node = new BVHBuildNode();

		AABB bbox;
		for (u32 i = start; i < end; ++i)
		{
			bbox = AABB::merge(bbox, buildData[i].bounds);
		}

		u32 primCount = end - start;
		if (primCount == 1)
		{
			/*u32 firstPrimOffset = orderedPrims.size();
			for (u32 i = start; i < end; ++i)
			{
			u32 primNumber = buildData[i].primitiveNumber;
			orderedPrims.push_back(m_primitives[primNumber]);
			}
			node->initLeaf(firstPrimOffset, primCount, bbox);*/
			createLeaf(orderedPrims, start, end, buildData, primitives, node, primCount, bbox);
		}
		else
		{
			AABB centroidBounds;
			for (u32 i = start; i < end; ++i)
			{
				centroidBounds = AABB::merge(centroidBounds, buildData[i].centroid);
			}
			u32 dim = centroidBounds.maximumExtend();

			u32 mid = (start + end) / 2;
			if (centroidBounds.max[dim] == centroidBounds.min[dim])
			{
				/*u32 firstPrimOffset = orderedPrims.size();
				for (u32 i = start; i < end; ++i)
				{
				u32 primNumber = buildData[i].primitiveNumber;
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
				const u32 bucketCount = 12;
				BucketInfo buckets[bucketCount];

				// initialize BucketInfo
				for (u32 i = start; i < end; ++i)
				{
					u32 b = bucketCount * ((buildData[i].centroid[dim] - centroidBounds.min[dim]) /
						(centroidBounds.max[dim] - centroidBounds.min[dim]));

					if (b == bucketCount)b = bucketCount - 1;
					++buckets[b].count;
					buckets[b].bounds = AABB::merge(buckets[b].bounds, buildData[i].bounds);
				}

				// compute costs
				f32 cost[bucketCount - 1];
				for (u32 i = 0; i < bucketCount - 1; ++i)
				{
					AABB b0, b1;
					u32 count0 = 0, count1 = 0;
					for (u32 j = 0; j <= i; ++j)
					{
						b0 = AABB::merge(b0, buckets[j].bounds);
						count0 += buckets[j].count;
					}
					for (u32 j = i + 1; j < bucketCount; ++j)
					{
						b1 = AABB::merge(b1, buckets[j].bounds);
						count1 += buckets[j].count;
					}
					cost[i] = .125f + (count0 * b0.surfaceArea() + count1 * b1.surfaceArea()) / bbox.surfaceArea();
				}

				// find bucket to split
				f32 minCost = cost[0];
				u32 minSplitCost = 0;
				for (u32 i = 1; i < bucketCount - 1; ++i)
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

	u32 flattenBVHTree(LinearBVHNode* pNodes, BVHBuildNode* node, u32* offset)
	{
		LinearBVHNode* linearNode = &pNodes[*offset];
		linearNode->bounds = node->bounds;
		u32 myOffset = (*offset)++;
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

void BVH::create(const Primitive* primitives, u32 primitiveCount)
{
	m_maxPrimsInNode = std::min(255u, primitiveCount);

	std::vector<BVHPrimitiveInfo> buildData;
	buildData.reserve(primitiveCount);
	m_primitives.reserve(primitiveCount);
	for (u32 i = 0; i < primitiveCount; ++i)
	{
		auto bounds = primitives[i].worldBound();

		buildData.push_back(BVHPrimitiveInfo(i, bounds));

		m_primitives.push_back(&primitives[i]);
	}

	u32 totalNodes = 0;
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

	m_pNodes = vx::make_unique<LinearBVHNode[]>(totalNodes);
	u32 offset = 0;
	flattenBVHTree(m_pNodes.get(), root, &offset);
}

bool BVH::intersect(const Ray &ray) const
{
	if (m_pNodes == nullptr)
		return false;

	bool hit = false;
	//vx::float3 origin = ray(ray.mint);
	vx::float3 invDir = 1.0f / ray.d;
	u32 dirIsNeg[3] = { invDir.x < 0, invDir.y < 0, invDir.z < 0 };

	u32 todoOffset = 0, nodeNum = 0;
	u32 todo[64];
	while (true)
	{
		auto node = &m_pNodes[nodeNum];

		if (intersectP(node->bounds, ray, invDir, dirIsNeg))
		{
			if (node->primCount > 0)
			{
				// intersect ray against primitives
				for (u32 i = 0; i < node->primCount; ++i)
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

bool BVH::intersectP(const AABB &bounds, const Ray &ray, const vx::float3 &invDir, const u32 dirIsNeg[3]) const
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