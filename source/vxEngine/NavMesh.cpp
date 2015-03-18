#include "NavMesh.h"
#include "yamlHelper.h"
#include "utility.h"
#include "File.h"

namespace YAML
{
	template<>
	struct convert<AABB>
	{
		static Node encode(const AABB &rhs)
		{
			Node n;
			n["min"] = rhs.min;
			n["max"] = rhs.max;

			return n;
		}

		static bool decode(const Node &node, AABB &v)
		{
			if (node.size() != 2 && !node.IsSequence())
				return false;

			v.min = node["min"].as<vx::float3>();
			v.max = node["max"].as<vx::float3>();

			return true;
		}
	};
}

NavMesh::NavMesh(NavMesh &&rhs)
	:m_vertices(std::move(rhs.m_vertices)),
	m_indices(std::move(rhs.m_indices)),
	m_bounds(rhs.m_bounds),
	m_vertexCount(rhs.m_vertexCount),
	m_indexCount(rhs.m_indexCount)
{
}

NavMesh& NavMesh::operator = (NavMesh &&rhs)
{
	if (this != &rhs)
	{
		std::swap(m_vertices, rhs.m_vertices);
		std::swap(m_indices, rhs.m_indices);
		std::swap(m_bounds, rhs.m_bounds);
		std::swap(m_vertexCount, rhs.m_vertexCount);
		std::swap(m_indexCount, rhs.m_indexCount);
	}

	return *this;
}

void NavMesh::loadFromYAML(const YAML::Node &node)
{
	auto vertices = node["vertices"].as < std::vector<vx::float3>>();
	auto indices = node["indices"].as < std::vector<U16>>();

	m_bounds = node["bounds"].as<AABB>();

	m_vertexCount = vertices.size();
	m_indexCount = indices.size();

	m_vertices = std::make_unique<vx::float3[]>(m_vertexCount);
	m_indices = std::make_unique<U16[]>(m_indexCount);

	vx::memcpy(m_vertices.get(), vertices.data(), m_vertexCount);
	vx::memcpy(m_indices.get(), indices.data(), m_indexCount);
}

void NavMesh::saveToFile(File *file) const
{
	file->write(m_vertexCount);
	file->write(m_indexCount);
	file->write(m_bounds);

	file->write(m_vertices.get(), m_vertexCount);
	file->write(m_indices.get(), m_indexCount);
}

const U8* NavMesh::load(const U8 *ptr)
{
	ptr = vx::read(m_vertexCount, ptr);
	ptr = vx::read(m_indexCount, ptr);
	ptr = vx::read(m_bounds, ptr);

#warning Please use a custom allocator !

	m_vertices = std::make_unique<vx::float3[]>(m_vertexCount);
	m_indices = std::make_unique<U16[]>(m_indexCount);

	ptr = vx::read(m_vertices.get(), ptr, m_vertexCount);
	ptr = vx::read(m_indices.get(), ptr, m_indexCount);

	return ptr;
}