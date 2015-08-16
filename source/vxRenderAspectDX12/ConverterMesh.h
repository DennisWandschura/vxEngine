#pragma once

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