#pragma once

#include "RenderAspect.h"
#include "EditorRenderData.h"

class EditorRenderAspect : public RenderAspect
{
	enum class EditorUpdate : U32;

	Editor::RenderData m_editorData;
	std::mutex m_updateDataMutex{};
	std::atomic_uint m_updateEditor{ 0 };
	std::vector<std::pair<Variant, EditorUpdate>> m_updateData;

	void addMesh(const vx::StringID64 &sid);
	void addMaterial(const vx::StringID64 &sid);
	void addMeshInstanceToBuffers();
	void updateInstance(const vx::StringID64 &sid);
	void updateEditor();

	void handleEditorEvent(const Event &evt);

public:
	EditorRenderAspect(Logfile &logfile, FileAspect &fileAspect);

	bool initialize(const std::string &dataDir, HWND panel, HWND tmp, const vx::uint2 &windowResolution, F32 fovDeg, F32 zNear, F32 zFar, bool vsync, bool debug,
		vx::StackAllocator *pAllocator);

	void update();
	void render();

	void editor_addMesh(const vx::StringID64 &sid, const char* name, const vx::Mesh* pMesh);
	void editor_addMaterial(const vx::StringID64 &sid, const char* name, Material* pMaterial);
	// returns 1 on success, 0 on failure
	U8 editor_addMeshInstance(const vx::StringID64 instanceSid, const vx::StringID64 meshSid, const vx::StringID64 materialSid, const vx::Transform &transform);
	U32 editor_getTransform(const vx::StringID64 instanceSid, vx::float3 &translation, vx::float3 &rotation, F32 &scaling);
	void editor_updateTranslation(const vx::StringID64 instanceSid, const vx::float3 &translation);

	void editor_moveCamera(F32 dirX, F32 dirY, F32 dirZ);
	void VX_CALLCONV editor_rotateCamera(const __m128 rotation);

	void editor_updateMouseHit(const vx::float3 &p);
	void editor_updateWaypoint(U32 offset, U32 count, const Waypoint* src);

	void handleEvent(const Event &evt) override;
};