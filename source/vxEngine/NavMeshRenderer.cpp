#include "NavMeshRenderer.h"
#include <vxLib/gl/StateManager.h>
#include <vxLib/gl/gl.h>
#include "NavGraph.h"
#include "NavMesh.h"
#include "utility.h"
#include "NavNode.h"
#include "NavConnection.h"
#include <vxLib/gl/ProgramPipeline.h>
#include <vxLib/gl/ShaderManager.h>

void NavMeshRenderer::createNavMesh()
{
	vx::gl::BufferDescription iboDesc;
	iboDesc.bufferType = vx::gl::BufferType::Element_Array_Buffer;
	iboDesc.size = sizeof(U16) * s_maxNavMeshIndices;
	iboDesc.flags = vx::gl::BufferStorageFlags::Write;
	iboDesc.immutable = 1;
	m_pColdData->m_navmeshIbo.create(iboDesc);

	vx::gl::BufferDescription vboDesc;
	vboDesc.bufferType = vx::gl::BufferType::Array_Buffer;
	vboDesc.size = sizeof(vx::float3) * s_maxNavMeshVertices;
	vboDesc.flags = vx::gl::BufferStorageFlags::Write;
	vboDesc.immutable = 1;
	m_pColdData->m_navmeshVbo.create(vboDesc);

	m_navmeshVao.create();
	m_navmeshVao.enableArrayAttrib(0);
	m_navmeshVao.arrayAttribFormatF(0, 3, 0, 0);
	m_navmeshVao.arrayAttribBinding(0, 0);
	m_navmeshVao.bindVertexBuffer(m_pColdData->m_navmeshVbo, 0, 0, sizeof(vx::float3));
	m_navmeshVao.bindIndexBuffer(m_pColdData->m_navmeshIbo);
}

void NavMeshRenderer::createNavNodes()
{
	{
		U16 indices[s_maxNavMeshIndices];
		for (U32 i = 0; i < s_maxNavMeshIndices; ++i)
		{
			indices[i] = i;
		}

		vx::gl::BufferDescription iboDesc;
		iboDesc.bufferType = vx::gl::BufferType::Element_Array_Buffer;
		iboDesc.size = sizeof(U16) * s_maxNavMeshIndices;
		iboDesc.immutable = 1;
		iboDesc.pData = indices;
		m_pColdData->m_navNodesIbo.create(iboDesc);
	}

	{
		vx::gl::BufferDescription vboDesc;
		vboDesc.bufferType = vx::gl::BufferType::Array_Buffer;
		vboDesc.size = sizeof(vx::float3) * s_maxNavMeshVertices;
		vboDesc.flags = vx::gl::BufferStorageFlags::Write;
		vboDesc.immutable = 1;
		m_pColdData->m_navNodesVbo.create(vboDesc);
	}

	m_navNodesVao.create();
	m_navNodesVao.enableArrayAttrib(0);
	m_navNodesVao.arrayAttribFormatF(0, 3, 0, 0);
	m_navNodesVao.arrayAttribBinding(0, 0);
	m_navNodesVao.bindVertexBuffer(m_pColdData->m_navNodesVbo, 0, 0, sizeof(vx::float3));
	m_navNodesVao.bindIndexBuffer(m_pColdData->m_navNodesIbo);
}

void NavMeshRenderer::createNavConnections()
{
	vx::gl::BufferDescription iboDesc;
	iboDesc.bufferType = vx::gl::BufferType::Element_Array_Buffer;
	iboDesc.size = sizeof(U16) * s_maxNavMeshIndices * 2;
	iboDesc.flags = vx::gl::BufferStorageFlags::Write;
	iboDesc.immutable = 1;
	m_pColdData->m_navConnectionsIbo.create(iboDesc);

	m_navConnectionsVao.create();
	m_navConnectionsVao.enableArrayAttrib(0);
	m_navConnectionsVao.arrayAttribFormatF(0, 3, 0, 0);
	m_navConnectionsVao.arrayAttribBinding(0, 0);
	m_navConnectionsVao.bindVertexBuffer(m_pColdData->m_navNodesVbo, 0, 0, sizeof(vx::float3));
	m_navConnectionsVao.bindIndexBuffer(m_pColdData->m_navConnectionsIbo);
}

NavMeshRenderer::NavMeshRenderer()
{

}

NavMeshRenderer::~NavMeshRenderer()
{

}

void NavMeshRenderer::initialize(const vx::gl::ShaderManager &shaderManager)
{
	m_pNavMeshPipe = shaderManager.getPipeline("navmesh.pipe");
	m_pColdData = std::make_unique<ColdData>();

	createNavMesh();
	createNavNodes();
	createNavConnections();
}

void NavMeshRenderer::render()
{
	// nav mesh
	/*m_stateManager.enable(vx::gl::Capabilities::Blend);
	m_stateManager.disable(vx::gl::Capabilities::Depth_Test);

	auto pPipeline = m_shaderManager.getPipeline("navmesh.pipe");
	vx::float3 color(0, 0, 1);
	glProgramUniform3fv(pPipeline->getFragmentShader(), 0, 1, color);

	m_stateManager.bindPipeline(pPipeline->getId());
	m_stateManager.bindVertexArray(m_navmeshVao.getId());
	glDrawElements(GL_TRIANGLES, m_navmeshIndexCount, GL_UNSIGNED_SHORT, 0);

	m_stateManager.disable(vx::gl::Capabilities::Blend);
	m_stateManager.enable(vx::gl::Capabilities::Depth_Test);*/

	vx::gl::StateManager::enable(vx::gl::Capabilities::Blend);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);

	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPointSize(3.f);

	//auto pPipeline = m_shaderManager.getPipeline("navmesh.pipe");

	vx::float3 color(1, 0, 0);
	glProgramUniform3fv(m_pNavMeshPipe->getFragmentShader(), 0, 1, color);

	vx::gl::StateManager::bindPipeline(m_pNavMeshPipe->getId());
	vx::gl::StateManager::bindVertexArray(m_navNodesVao.getId());
	glDrawElements(GL_POINTS, m_navNodesCount, GL_UNSIGNED_SHORT, 0);

	// connections
	vx::gl::StateManager::bindVertexArray(m_navConnectionsVao.getId());
	glDrawElements(GL_LINES, m_navConnectionCount, GL_UNSIGNED_SHORT, 0);

	vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);
}

void NavMeshRenderer::updateNavMeshBuffer(const NavMesh &navMesh)
{
	auto pVertices = navMesh.getVertices();
	auto pIndices = navMesh.getIndices();

	auto vertexCount = navMesh.getVertexCount();
	auto indexCount = navMesh.getIndexCount();

	auto p = m_pColdData->m_navmeshVbo.map<vx::float3>(vx::gl::Map::Write_Only);
	vx::memcpy(p.get(), pVertices, vertexCount);
	p.unmap();

	auto pi = m_pColdData->m_navmeshIbo.map<U16>(vx::gl::Map::Write_Only);
	vx::memcpy(pi.get(), pIndices, indexCount);
	pi.unmap();

	m_navmeshIndexCount = indexCount;
}

void NavMeshRenderer::updateBuffer(const NavGraph &navGraph)
{
	auto nodeCount = navGraph.getNodeCount();
	auto pNodes = navGraph.getNodes();

	auto ptrNodes = std::make_unique<vx::float3[]>(nodeCount);

	for (U32 i = 0; i < nodeCount; ++i)
	{
		auto &node = pNodes[i];
		ptrNodes[i] = node.m_position;
	}

	auto p = m_pColdData->m_navNodesVbo.map<vx::float3>(vx::gl::Map::Write_Only);
	vx::memcpy(p.get(), ptrNodes.get(), nodeCount);
	p.unmap();

	m_navNodesCount = nodeCount;

	auto connectionCount = navGraph.getConnectionCount();
	auto pConnections = navGraph.getConnections();

	auto connectionIndexCount = connectionCount * 2;
	VX_ASSERT(connectionIndexCount <= s_maxNavMeshIndices * 2);
	auto ptrConnections = std::make_unique<U16[]>(connectionIndexCount);

	U32 j = 0;
	for (U32 i = 0; i < connectionCount; ++i)
	{
		auto &connection = pConnections[i];

		ptrConnections[j++] = connection.m_fromNode;
		ptrConnections[j++] = connection.m_toNode;
	}

	VX_ASSERT(j == connectionIndexCount);

	auto pGpuConnections = m_pColdData->m_navConnectionsIbo.map<U16>(vx::gl::Map::Write_Only);
	vx::memcpy(pGpuConnections.get(), ptrConnections.get(), connectionIndexCount);
	pGpuConnections.unmap();

	m_navConnectionCount = connectionIndexCount;
}