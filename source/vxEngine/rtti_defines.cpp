#include "rtti.h"
#include <vxLib\math\Vector.h>
#include <vxLib/Graphics/Mesh.h>

RTTI_TYPE(bool){}

RTTI_TYPE(I8){}
RTTI_TYPE(I16){}
RTTI_TYPE(I32){}

RTTI_TYPE(U8){}
RTTI_TYPE(U16){}
RTTI_TYPE(U32){}

RTTI_TYPE(F32){}
RTTI_TYPE(F64){}

RTTI_TYPE(__m128){}

RTTI_TYPE(vx::uint2)
{
	RTTI_MEMBER(x);
	RTTI_MEMBER(y);
}

RTTI_TYPE(vx::float2)
{
	RTTI_MEMBER(x);
	RTTI_MEMBER(y);
}

RTTI_TYPE(vx::float3)
{
	RTTI_MEMBER(x);
	RTTI_MEMBER(y);
	RTTI_MEMBER(z);
}

RTTI_TYPE(vx::float4)
{
	RTTI_MEMBER(x);
	RTTI_MEMBER(y);
	RTTI_MEMBER(z);
	RTTI_MEMBER(w);
}

RTTI_TYPE(vx::Mesh){}