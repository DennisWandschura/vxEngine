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

#include <vxEngineLib/EditorRenderAspectInterface.h>
#include <vxLib/Graphics/Camera.h>
#include "gl/ObjectManager.h"
#include <vxGL/RenderContext.h>
#include "Graphics/CommandList.h"
#include <vxGL/ShaderManager.h>
#include <vxLib/Allocator/StackAllocator.h>
#include "MaterialManager.h"
#include "MeshManager.h"

namespace Editor
{
	struct VertexPositionColor;

	class RenderAspect : public Editor::RenderAspectInterface
	{
		struct ColdData;

		struct SelectedMeshInstance
		{
			const Editor::MeshInstance* ptr{ nullptr };
			vx::gl::DrawElementsIndirectCommand cmd;
		};

		Graphics::CommandList m_commandList;
		vx::gl::RenderContext m_renderContext;
		vx::gl::Buffer m_cameraBuffer;
		vx::Camera m_camera;
		gl::ObjectManager m_objectManager;
		MaterialManager m_materialManager;
		MeshManager m_meshManager;
		vx::gl::ShaderManager m_shaderManager;
		SelectedMeshInstance m_selectedInstance;
		vx::mat4 m_projectionMatrix;
		vx::StackAllocator m_allocator;
		vx::StackAllocator m_scratchAllocator;
		ResourceAspectInterface* m_resourceAspect;
		std::unique_ptr<ColdData> m_coldData;
		vx::float2 m_resolution;

		void updateCamera();
		void updateNavMeshIndexBuffer(const u16* indices, u32 triangleCount);
		void updateNavMeshIndexBuffer(const NavMesh &navMesh);
		void updateNavMeshVertexBufferWithSelectedVertex(const vx::float3* vertices, u32 count, u32(&selectedVertexIndex)[3], u8 selectedCount);
		void uploadToNavMeshVertexBuffer(const VertexPositionColor* vertices, u32 count);

		void createNavMeshNodesVao();
		void createNavMeshVao();
		void createNavMeshVertexVao();
		void createIndirectCmdBuffers();
		void createVoxelSegment();
		void createCommandList();
		bool createEditorTextures();

		void handleLoadScene(const vx::Message &evt);
		void handleFileEvent(const vx::Message &evt);

		void bindBuffers();

		void reportLiveObjects();

	public:
		RenderAspect();
		~RenderAspect();

		RenderAspectInitializeError initialize(const RenderAspectDescription &desc) override;
		void shutdown(void* hwnd);

		bool initializeProfiler(Logfile* errorlog);

		void makeCurrent(bool b);

		void queueUpdateTask(const RenderUpdateTaskType type, const u8* data, u32 dataSize) override;
		void queueUpdateCamera(const RenderUpdateCameraData &data);
		void update();

		void updateProfiler(f32 dt);

		void submitCommands();
		void endFrame();

		void handleMessage(const vx::Message &evt) override;

		void keyPressed(u16 key);

		void getTotalVRam(u32* totalVram) const;
		void getTotalAvailableVRam(u32* totalAvailableVram) const;
		void getAvailableVRam(u32* availableVram) const;

		void addMeshInstance(const Editor::MeshInstance &instance);
		bool removeMeshInstance(const vx::StringID &sid);

		bool setSelectedMeshInstance(const Editor::MeshInstance* instance);
		void setSelectedMeshInstanceTransform(vx::Transform &transform);
		bool setSelectedMeshInstanceMaterial(const Material* material);
		bool setMeshInstanceMesh(const vx::StringID &sid, const vx::StringID &meshSid);

		void moveCamera(f32 dirX, f32 dirY, f32 dirZ);
		void VX_CALLCONV rotateCamera(const __m128 rotation);

		void updateNavMeshBuffer(const NavMesh &navMesh, u32(&selectedVertex)[3], u8 selectedCount);
		void updateNavMeshBuffer(const NavMesh &navMesh);
		void updateInfluenceCellBuffer(const InfluenceMap &influenceMap);
		void updateNavMeshGraphNodesBuffer(const NavMeshGraph &navMeshGraph);
		void updateLightBuffer(const Light* lights, u32 count);
		void updateWaypoints(const Waypoint* w, u32 count);
		void updateSpawns(const Spawn* spawns, u32 count);
		void updateJoints(const Joint* joints, u32 count, const vx::sorted_vector<vx::StringID, MeshInstance> &meshinstances);

		const vx::Camera& getCamera() const;

		void getProjectionMatrix(vx::mat4* m) const override;
	};
}