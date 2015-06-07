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

namespace Editor
{
	class MeshInstance;
}

class NavMesh;
struct VertexNavMesh;
class InfluenceMap;
class NavMeshGraph;

#include "RenderAspect.h"
#include <vxLib/Variant.h>
#include <atomic>
#include "Graphics/CommandList.h"

class EditorRenderAspect : public RenderAspect
{
	enum class EditorUpdate : u32;

	struct SelectedMeshInstance
	{
		const Editor::MeshInstance* ptr{nullptr};
		vx::gl::DrawElementsIndirectCommand cmd;
	};

	struct EditorColdData
	{
		vx::gl::Texture m_editorTextures;

		u32 m_lightCount{ 0 };
		u32 m_navMeshIndexCount{ 0 };
		u32 m_navMeshVertexCount{ 0 };
	};

	Graphics::CommandList m_commandList;

	u32 m_navMeshGraphNodesCount{0};

	SelectedMeshInstance m_selectedInstance{};
	std::mutex m_updateDataMutex{};
	std::atomic_uint m_updateEditor{ 0 };
	std::vector<std::pair<vx::Variant, EditorUpdate>> m_updateData;
	std::unique_ptr<EditorColdData> m_pEditorColdData;

	void createNavMeshVao();
	void createNavMeshVertexVao();

	void createNavMeshNodesVao();

	void createIndirectCmdBuffers();

	void createCommandList();

	void createEditorTextures();

	void handleFileEvent(const vx::Event &evt);

	void addMesh(const vx::StringID &sid);
	void addMaterial(const vx::StringID &sid);
	void addMeshInstanceToBuffers();
	void updateInstance(const vx::StringID &sid);
	void updateEditor();

	void handleEditorEvent(const vx::Event &evt);
	void handleLoadScene(const vx::Event &evt);
	void handleLoadMesh(const vx::Event &evt);

	void updateCamera();

	void uploadToNavMeshVertexBuffer(const VertexNavMesh* vertices, u32 count);
	void updateNavMeshVertexBufferWithSelectedVertex(const vx::float3* vertices, u32 count, u32(&selectedVertexIndex)[3], u8 selectedCount);
	void updateNavMeshIndexBuffer(const u16* indices, u32 count);
	void updateNavMeshIndexBuffer(const NavMesh &navMesh);

public:
	EditorRenderAspect();

	bool initialize(const std::string &dataDir, HWND panel, HWND tmp, vx::StackAllocator *pAllocator, const EngineConfig* settings);

	void update();
	void render();

	void editor_moveCamera(f32 dirX, f32 dirY, f32 dirZ);
	void VX_CALLCONV editor_rotateCamera(const __m128 rotation);

	void handleEvent(const vx::Event &evt) override;

	const vx::Camera& getCamera() const;

	bool setSelectedMeshInstance(const Editor::MeshInstance* instance);
	void setSelectedMeshInstanceTransform(vx::Transform &transform);
	bool setSelectedMeshInstanceMaterial(const Material* material) const;
	bool setMeshInstanceMesh(const vx::StringID &sid, const vx::StringID &meshSid);

	void editorAddMeshInstance(const Editor::MeshInstance &instance);
	bool removeSelectedMeshInstance();

	void updateNavMeshBuffer(const NavMesh &navMesh, u32(&selectedVertex)[3], u8 selectedCount);
	void updateNavMeshBuffer(const NavMesh &navMesh);

	void updateInfluenceCellBuffer(const InfluenceMap &influenceMap);

	void updateNavMeshGraphNodesBuffer(const NavMeshGraph &navMeshGraph);

	void updateLightBuffer(const Light* lights, u32 count);

	void showNavmesh(bool b);
	void showInfluenceMap(bool b);
};