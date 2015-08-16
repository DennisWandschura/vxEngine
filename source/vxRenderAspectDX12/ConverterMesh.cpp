#include "ConverterMesh.h"
#include <vxLib/Graphics/Mesh.h>
#include <vector>
#include "AdjacencyTreeBuilder.h"

void CalculateQuaternionFromTBN(const vx::float3& t, const  vx::float3& b, const  vx::float3& n, Quaternion* result, bool* invertedHandedness)
{
	VX_ASSERT(invertedHandedness);// "Output param 'invertedHandedness' can't be NULL");
	VX_ASSERT(result);// "Output param 'result' can't be NULL");

	*result = Quaternion();

	//Do not construct quaternions from wrong basis
	unsigned long res = Quaternion::checkQuaternionSource(t, b, n);

	const float dotCross = vx::dot3(t, vx::cross(b, n));
	float handedness = dotCross > 0.0f ? 1.0f : -1.0f;

	mat3 tbn;// = Matrix4x3::Identity();
	tbn[0] = t;
	tbn[1] = b * vx::float3(handedness);
	tbn[2] = n;

	//Basis have scale, try to renormalize
	if ((res & Quaternion::SOURCE_BASIS_HAVE_SCALE) != 0)
	{
		//vx::float3 normalizedN = n;
		//normalizedN.Normalize();
		vx::float3 normalizedN = vx::normalize3(n);

		//Vector3 normalizedT = t;
		//normalizedT.Normalize();
		vx::float3 normalizedT = vx::normalize3(t);

		//tbn.SetX(normalizedT);
		//tbn.SetY(cross(normalizedT, normalizedN));
		//tbn.SetZ(normalizedN);
		tbn[0] = normalizedT;
		tbn[1] = vx::cross(normalizedT, normalizedN);
		tbn[2] = normalizedN;
	}

	*result = Quaternion(tbn);

	if (handedness < 0.0)
		*invertedHandedness = true;
	else
		*invertedHandedness = false;
}

void ConverterMesh::convertMesh(const vx::Mesh &mesh, MeshQ* meshQ)
{
	auto indexCount = mesh.getIndexCount();
	auto indices = mesh.getIndices();
	auto vertices = mesh.getVertices();
	auto vertexCount = mesh.getVertexCount();

	//auto triangleCount = indexCount / 3;

	auto newVertices = std::make_unique<VertexQ[]>(vertexCount);
	for (u32 i = 0; i < vertexCount; ++i)
	{
		auto &v = vertices[i];

		bool invertedH = false;
		Quaternion q;
		CalculateQuaternionFromTBN(v.tangent, v.bitangent, v.normal,&q, &invertedH);

		newVertices[i].p = v.position;
		newVertices[i].q = q;

	}

	std::vector<u32> tmpIndices;
	tmpIndices.reserve(indexCount);
	for (u32 i = 0; i < indexCount; ++i)
	{
		tmpIndices.push_back(indices[i]);
	}

	AdjacencyTreeBuilder<u32> adjacencyBuilder;
	adjacencyBuilder.Build(tmpIndices, indexCount);

	const std::vector<AdjacencyTreeBuilder<u32>::Edge> & adjacencies = adjacencyBuilder.GetAdjacencies();

	//Traverse the tree, align quaternions so their dot product becomes positive
	for (size_t i = 0; i < adjacencies.size(); ++i)
	{
		auto &child = newVertices[adjacencies[i].first].q;
		auto &parent = newVertices[adjacencies[i].second].q;

		if (dot(child, parent) < 0.0f)
		{
			child.x = -child.x;
			child.y = -child.y;
			child.z = -child.z;
			child.w = -child.w;
		}
	}

	auto newIndices = std::make_unique<u32[]>(indexCount);
	memcpy(newIndices.get(), tmpIndices.data(), sizeof(u32) * indexCount);

	meshQ->vertices.swap(newVertices);
	meshQ->indices.swap(newIndices);
	meshQ->vertexCount = vertexCount;
	meshQ->indexCount = indexCount;
	
	/*for (u32 i = 0, j = 0; i < indexCount; i += 3, ++j)
	{
		auto i0 = indices[i + 0];
		auto i1 = indices[i + 1];
		auto i2 = indices[i + 2];

		auto v0 = vertices[i0];
		auto v1 = vertices[i1];
		auto v2 = vertices[i2];

		v0.bitangent = vx::cross(v0.normal, v0.tangent);
		v1.bitangent = vx::cross(v1.normal, v1.tangent);
		v2.bitangent = vx::cross(v2.normal, v2.tangent);

		f32 basisCheck0 = vx::dot3(vx::cross(v0.tangent, v0.bitangent), v0.normal);
		f32 basisCheck1 = vx::dot3(vx::cross(v1.tangent, v1.bitangent), v1.normal);
		f32 basisCheck2 = vx::dot3(vx::cross(v2.tangent, v2.bitangent), v2.normal);

		mat3 tbn;

		tbn[0] = v0.tangent;
		tbn[1] = v0.bitangent * basisCheck0;
		tbn[2] = v0.normal;

		Quaternion q0(tbn);

		tbn[0] = v1.tangent;
		tbn[1] = v1.bitangent * basisCheck1;
		tbn[2] = v1.normal;
		Quaternion q1(tbn);

		tbn[0] = v2.tangent;
		tbn[1] = v2.bitangent * basisCheck2;
		tbn[2] = v2.normal;
		Quaternion q2(tbn);

		triangles[j].v[0].p = v0.position;
		triangles[j].v[0].q = q0;

		triangles[j].v[1].p = v1.position;
		triangles[j].v[1].q = q1;

		triangles[j].v[2].p = v2.position;
		triangles[j].v[2].q = q2;
	}

	triangleMesh->triangles.swap(triangles);
	triangleMesh->count = triangleCount;*/
}