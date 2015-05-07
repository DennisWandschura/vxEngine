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
		u32 m_waypointCount{0};
		vx::gl::VertexArray m_mouseHitVao;
		vx::gl::Buffer m_editorWaypointsVbo;
		vx::gl::Buffer m_editorWaypointsIbo;
		vx::gl::Buffer m_mouseHitVertex;
		vx::sorted_vector<vx::StringID, MeshEntry> m_editorMeshEntries;
		vx::sorted_vector<vx::StringID, u32> m_editorMeshInstanceIndices;
		vx::sorted_vector<const Material*, u32> m_editorMaterialIndices;
		u32 m_editorVertexCount{ 0 };
		u32 m_editorIndexCount{ 0 };
		u32 m_editorMaterialCount{ 0 };
		u32 m_editorMeshInstanceCount{ 0 };

		void createWaypointBuffer();

	public:
		RenderData();
		~RenderData();

		void initialize();

		void drawMouse(const vx::gl::ShaderManager &shaderManager) const;
		void drawWaypoints(const vx::gl::ShaderManager &shaderManager) const;

		void addMesh(vx::StringID sid);
		u32 addMaterial(vx::StringID sid, const Material* p);

		void updateMouseHit(const vx::float3 &p);
		void updateWaypoint(u32 offset, u32 count, const Waypoint* src);

		u8 getMeshInstanceId(vx::StringID sid, u32* id);
	};
}