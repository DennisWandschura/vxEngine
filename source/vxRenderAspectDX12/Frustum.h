#pragma once

#include <vxLib/math/matrix.h>

class Frustum
{
	struct PackedPlane
	{
		__m128 nd;
	};

	enum FrustumPlane { FrustumPlaneNear, FrustumPlaneLeft, FrustumPlaneRight};

	PackedPlane m_planes[3];

public:
	Frustum();
	~Frustum();

	void update(const vx::mat4 &invPvMatrix);

	u32 __vectorcall testSpheres(__m128* cr, u32 count, u32* indices) const;
};