#pragma once

#include <vxLib/gl/Buffer.h>
#include <vxLib/gl/VertexArray.h>
#include <vxLib/Container/sorted_vector.h>
#include <vxLib/StringID.h>
#include <vxLib/math/Vector.h>

class Material;
struct MeshEntry;
struct Waypoint;

namespace vx
{
	namespace gl
	{
		class ShaderManager;
	}
}

namespace Editor
{
	class RenderData
	{
		static const auto s_maxWaypoints = 100u;

		vx::gl::VertexArray m_waypointsVao;
		U32 m_waypointCount{0};
		vx::gl::VertexArray m_mouseHitVao;
		vx::gl::Buffer m_editorWaypointsVbo;
		vx::gl::Buffer m_editorWaypointsIbo;
		vx::gl::Buffer m_mouseHitVertex;
		vx::sorted_vector<vx::StringID64, MeshEntry> m_editorMeshEntries;
		vx::sorted_vector<vx::StringID64, U32> m_editorMeshInstanceIndices;
		vx::sorted_vector<const Material*, U32> m_editorMaterialIndices;
		U32 m_editorVertexCount{ 0 };
		U32 m_editorIndexCount{ 0 };
		U32 m_editorMaterialCount{ 0 };
		U32 m_editorMeshInstanceCount{ 0 };

		void createWaypointBuffer();

	public:
		void initialize();

		void drawMouse(const vx::gl::ShaderManager &shaderManager) const;
		void drawWaypoints(const vx::gl::ShaderManager &shaderManager) const;

		void addMesh(const vx::StringID64 &sid);
		U32 addMaterial(const vx::StringID64 &sid, const Material* p);

		void updateMouseHit(const vx::float3 &p);
		void updateWaypoint(U32 offset, U32 count, const Waypoint* src);

		U8 getMeshInstanceId(const vx::StringID64 &sid, U32* id);
	};
}