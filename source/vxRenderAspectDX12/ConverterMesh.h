#pragma once

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

namespace vx
{
	class Mesh;
}

#include "Quaternion.h"
#include <memory>

struct VertexQ
{
	vx::float3 p;
	Quaternion q;
};

struct VertexTriangle
{
	VertexQ v[3];
};

struct TriangleMesh
{
	std::unique_ptr<VertexTriangle[]> triangles;
	u32 count;
};

struct MeshQ
{
	std::unique_ptr<VertexQ[]> vertices;
	std::unique_ptr<u32[]> indices;
	u32 vertexCount;
	u32 indexCount;
};

struct ConverterMesh
{
	static void convertMesh(const vx::Mesh &mesh, MeshQ* meshQ);
};