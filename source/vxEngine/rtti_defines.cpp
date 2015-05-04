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