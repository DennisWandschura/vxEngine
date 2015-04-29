#pragma once

class Ray;

#include "AABB.h"

class Primitive
{
public:
	virtual const AABB& worldBound() const = 0;

	virtual bool intersects(const Ray &ray) const = 0;
};