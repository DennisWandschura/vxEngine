#pragma once

class Material;

namespace vx
{
	class Mesh;
}

namespace YAML
{
	class Node;
}

#include <vxLib/StringID.h>
#include "Transform.h"

class MeshInstance
{
	vx::StringID64 m_meshSid;
	vx::StringID64 m_materialSid;
	vx::Transform m_transform;

public:
	MeshInstance();
	MeshInstance(vx::StringID64 meshSid, vx::StringID64 materialSid, const vx::Transform &transform);

	vx::StringID64 getMeshSid() const noexcept{ return m_meshSid; }
	vx::StringID64 getMaterialSid() const noexcept{ return m_materialSid; }
	const vx::Transform& getTransform() const noexcept{ return m_transform; }

#if _VX_EDITOR
	void setTranslation(const vx::float3 &translation);
#endif
};

class MeshInstanceFile
{
	using Buffer = char[32];

	Buffer m_mesh;
	Buffer m_material;
	vx::Transform m_transform;

public:
	MeshInstanceFile();
	MeshInstanceFile(const char(&meshName)[32], const char(&materialName)[32], const vx::Transform &transform);

	void load(const U8 *ptr);
	void load(const YAML::Node &node);

	void save(YAML::Node &node) const;

	const char* getMeshFile() const noexcept;
	const char* getMaterialFile() const noexcept;
	const vx::Transform& getTransform() const noexcept{ return m_transform; }
};