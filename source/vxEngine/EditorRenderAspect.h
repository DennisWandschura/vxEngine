#pragma once

class MeshInstance;
class NavMesh;
struct VertexNavMesh;
class InfluenceMap;
struct TriangleIndices;
class NavMeshGraph;

#include "RenderAspect.h"
#include "EditorRenderData.h"
#include <vxLib/Variant.h>
#include <atomic>
#include "Graphics/CommandList.h"

typedef  struct {
	U32  count;
	U32  instanceCount;
	U32  first;
	U32  baseInstance;
} DrawArraysIndirectCommand;

class EditorRenderAspect : public RenderAspect
{
	enum class EditorUpdate : U32;

	struct SelectedMeshInstance
	{
		const MeshInstance* ptr{nullptr};
		vx::gl::DrawElementsIndirectCommand cmd;
	};

	struct EditorColdData
	{
		vx::gl::Texture m_editorTextures;
		vx::gl::Buffer m_editorTextureBuffer;
		vx::gl::Buffer m_spawnPointVbo;
		vx::gl::VertexArray m_spawnPointVao;
		vx::gl::Buffer m_spawnPointCmdBuffer;

		vx::gl::Buffer m_navMeshVertexVbo;
		vx::gl::Buffer m_navMeshVertexIbo;

		vx::gl::Buffer m_influenceCellVbo;

		vx::gl::Buffer m_navMeshGraphNodesVbo;

		vx::gl::Buffer m_navMeshVertexCmdBuffer;
		vx::gl::Buffer m_graphNodesCmdBuffer;
		vx::gl::Buffer m_navmeshCmdBuffer;
		vx::gl::Buffer m_lightCmdBuffer;
		vx::gl::Buffer m_influenceMapCmdBuffer;

		U32 m_influenceCellCount{ 0 };
		U32 m_lightCount{ 0 };
		U32 m_navMeshIndexCount{ 0 };
		U32 m_navMeshVertexCount{ 0 };
		vx::gl::VertexArray m_navMeshVao;
		vx::gl::VertexArray m_navMeshVertexVao;
		vx::gl::VertexArray m_influenceVao;
	};

	vx::gl::Buffer m_meshCountBuffer;
	Graphics::CommandList m_commandList;

	vx::gl::VertexArray m_navMeshGraphNodesVao;

	U32 m_navMeshGraphNodesCount{0};
	Editor::RenderData m_editorData;

	SelectedMeshInstance m_selectedInstance{};
	std::mutex m_updateDataMutex{};
	std::atomic_uint m_updateEditor{ 0 };
	std::vector<std::pair<vx::Variant, EditorUpdate>> m_updateData;
	std::unique_ptr<EditorColdData> m_pEditorColdData;

	void createNavMeshVao();
	void createNavMeshVertexVao();

	void createInfluenceCellVao();

	void createNavMeshNodesVao();

	void createIndirectCmdBuffers();

	void createCommandList();

	void createEditorTextures();

	void handleFileEvent(const Event &evt);

	void addMesh(const vx::StringID64 &sid);
	void addMaterial(const vx::StringID64 &sid);
	void addMeshInstanceToBuffers();
	void updateInstance(const vx::StringID64 &sid);
	void updateEditor();

	void handleEditorEvent(const Event &evt);
	void handleLoadScene(const Event &evt);

	void updateCamera();

	void uploadToNavMeshVertexBuffer(const VertexNavMesh* vertices, U32 count);
	void updateNavMeshVertexBufferWithSelectedVertex(const vx::float3* vertices, U32 count, U32(&selectedVertexIndex)[3], U8 selectedCount);
	void updateNavMeshIndexBuffer(const TriangleIndices* indices, U32 count);
	void updateNavMeshIndexBuffer(const NavMesh &navMesh);

public:
	EditorRenderAspect();

	bool initialize(const std::string &dataDir, HWND panel, HWND tmp, const vx::uint2 &windowResolution, F32 fovDeg, F32 zNear, F32 zFar, bool vsync, bool debug,
		vx::StackAllocator *pAllocator);

	void update();
	void render();

	void editor_moveCamera(F32 dirX, F32 dirY, F32 dirZ);
	void VX_CALLCONV editor_rotateCamera(const __m128 rotation);

	void editor_updateMouseHit(const vx::float3 &p);
	void editor_updateWaypoint(U32 offset, U32 count, const Waypoint* src);

	void handleEvent(const Event &evt) override;

	const vx::Camera& getCamera() const;

	bool setSelectedMeshInstance(const MeshInstance* p);
	void updateSelectedMeshInstanceTransform(vx::Transform &transform);

	void updateNavMeshBuffer(const NavMesh &navMesh, U32(&selectedVertex)[3], U8 selectedCount);
	void updateNavMeshBuffer(const NavMesh &navMesh);

	void updateInfluenceCellBuffer(const InfluenceMap &influenceMap);

	void updateNavMeshGraphNodesBuffer(const NavMeshGraph &navMeshGraph);
};