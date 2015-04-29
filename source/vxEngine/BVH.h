#pragma once

class Primitive;
struct AABB;
class Ray;
struct LinearBVHNode;

#include <vxLib/math/Vector.h>
#include <vector>
#include <memory>

class BVH
{
	std::unique_ptr<LinearBVHNode[]> m_pNodes;
	U32 m_primCount;
	U32 m_maxPrimsInNode;
	std::vector<const Primitive*> m_primitives;

	bool intersectP(const AABB &bounds, const Ray &ray, const vx::float3 &invDir, const U32 dirIsNeg[3]) const;

public:
	BVH();
	~BVH();

	void create(const Primitive* primitives, U32 primitiveCount);

	bool intersect(const Ray &ray) const;
};