#pragma once

class File;

namespace YAML
{
	class Node;
}

#include <vxLib/math/Vector.h>
#include <memory>
#include "AABB.h"

class NavMesh
{
	std::unique_ptr<vx::float3[]> m_vertices;
	std::unique_ptr<U16[]> m_indices;
	AABB m_bounds{};
	U32 m_vertexCount{0};
	U32 m_indexCount{0};

public:
	/////////////// constructors
	NavMesh() = default;
	NavMesh(const NavMesh &rhs) = delete;
	NavMesh(NavMesh &&rhs);
	/////////////// constructors

	/////////////// operators
	NavMesh& operator=(const NavMesh &rhs) = delete;
	NavMesh& operator=(NavMesh &&rhs);
	/////////////// operators

	/////////////// loading
	void loadFromYAML(const YAML::Node &node);
	const U8* load(const U8 *ptr);
	/////////////// loading

	/////////////// saving
	void saveToFile(File *file) const;
	/////////////// saving

	/////////////// getters
	const vx::float3* getVertices() const { return m_vertices.get(); }
	U32 getVertexCount() const { return m_vertexCount; }

	const U16* getIndices() const { return m_indices.get(); }
	U32 getIndexCount() const { return m_indexCount; }

	const AABB& getBounds() const { return m_bounds; }
	/////////////// getters
};