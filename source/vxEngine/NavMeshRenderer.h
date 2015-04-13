#pragma once

class NavMesh;
class NavGraph;

namespace vx
{
	namespace gl
	{
		class ShaderManager;
		class ProgramPipeline;
	}
}

#include <vxLib/gl/VertexArray.h>
#include <vxLib/gl/Buffer.h>
#include <memory>

class NavMeshRenderer
{
	static const auto s_maxNavMeshVertices = 100u;
	static const auto s_maxNavMeshIndices = 100u;

	struct ColdData
	{
		vx::gl::Buffer m_navmeshVbo;
		vx::gl::Buffer m_navmeshIbo;
		vx::gl::Buffer m_navNodesVbo;
		vx::gl::Buffer m_navNodesIbo;
		vx::gl::Buffer m_navConnectionsIbo;
	};

	const vx::gl::ProgramPipeline* m_pNavMeshPipe;
	vx::gl::VertexArray m_navNodesVao;
	vx::gl::VertexArray m_navmeshVao;
	vx::gl::VertexArray m_navConnectionsVao;
	U32 m_navNodesCount{ 0 };
	U32 m_navConnectionCount{ 0 };
	U32 m_navmeshIndexCount{ 0 };
	std::unique_ptr<ColdData> m_pColdData;

	void createNavMesh();
	void createNavNodes();
	void createNavConnections();

public:
	NavMeshRenderer();
	~NavMeshRenderer();

	void initialize(const vx::gl::ShaderManager &shaderManager);

	void render();

	void updateNavMeshBuffer(const NavMesh &navMesh);
	void updateBuffer(const NavGraph &navGraph);
};