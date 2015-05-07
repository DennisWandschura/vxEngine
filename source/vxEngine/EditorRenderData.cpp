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
#include "EditorRenderData.h"
#include <vxLib/gl/StateManager.h>
#include <vxLib/gl/gl.h>
#include <vxLib/gl/ShaderManager.h>
#include "Waypoint.h"
#include <vxLib/gl/ProgramPipeline.h>
#include "SceneRenderer.h"

namespace Editor
{
	RenderData::RenderData()
	{

	}

	RenderData::~RenderData()
	{

	}

	void RenderData::initialize()
	{
		vx::float3 pos(0, 5.0f, 0);
		vx::gl::BufferDescription mdesc;
		mdesc.bufferType = vx::gl::BufferType::Array_Buffer;
		mdesc.flags = vx::gl::BufferStorageFlags::Write;
		mdesc.size = sizeof(vx::float3);
		mdesc.pData = &pos;
		m_mouseHitVertex.create(mdesc);

		m_mouseHitVao.create();

		m_mouseHitVao.enableArrayAttrib(0);
		m_mouseHitVao.arrayAttribFormatF(0, 3, 0, 0);
		m_mouseHitVao.arrayAttribBinding(0, 0);
		m_mouseHitVao.bindVertexBuffer(m_mouseHitVertex, 0, 0, sizeof(vx::float3));

		createWaypointBuffer();
	}

	void RenderData::createWaypointBuffer()
	{
		vx::gl::BufferDescription vdesc;
		vdesc.bufferType = vx::gl::BufferType::Array_Buffer;
		vdesc.flags = vx::gl::BufferStorageFlags::Write;
		vdesc.size = sizeof(Waypoint) * s_maxWaypoints;
		m_editorWaypointsVbo.create(vdesc);

		u8 indices[s_maxWaypoints];
		for (auto i = 0u; i < s_maxWaypoints; ++i)
		{
			indices[i] = i;
		}

		vx::gl::BufferDescription idesc;
		idesc.bufferType = vx::gl::BufferType::Element_Array_Buffer;
		idesc.size = sizeof(u8) * s_maxWaypoints;
		idesc.pData = indices;
		m_editorWaypointsIbo.create(idesc);

		m_waypointsVao.create();
		m_waypointsVao.bindIndexBuffer(m_editorWaypointsIbo);

		m_waypointsVao.enableArrayAttrib(0);
		m_waypointsVao.arrayAttribFormatF(0, 4, 0, 0);
		m_waypointsVao.arrayAttribBinding(0, 0);
		m_waypointsVao.bindVertexBuffer(m_editorWaypointsVbo, 0, 0, sizeof(Waypoint));
	}

	void RenderData::drawMouse(const vx::gl::ShaderManager &shaderManager) const
	{
		vx::gl::StateManager::enable(vx::gl::Capabilities::Blend);
		vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);

		vx::gl::StateManager::bindVertexArray(m_mouseHitVao);
		glPointSize(5.0f);

		auto pPipeline = shaderManager.getPipeline("draw_point.pipe");
		vx::gl::StateManager::bindPipeline(pPipeline->getId());
		glDrawArrays(GL_POINTS, 0, 1);

		vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);
		vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);
	}

	void RenderData::drawWaypoints(const vx::gl::ShaderManager &shaderManager) const
	{
		vx::gl::StateManager::enable(vx::gl::Capabilities::Blend);
		vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);

		vx::gl::StateManager::bindVertexArray(m_waypointsVao);
		glPointSize(5.0f);

		auto pPipeline = shaderManager.getPipeline("draw_point.pipe");
		vx::gl::StateManager::bindPipeline(pPipeline->getId());
		glDrawElements(GL_POINTS, m_waypointCount, GL_UNSIGNED_BYTE, 0);

		vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);
		vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);
	}

	void RenderData::addMesh(vx::StringID sid)
	{
		VX_UNREFERENCED_PARAMETER(sid);
	}

	u32 RenderData::addMaterial(vx::StringID sid, const Material* pMaterial)
	{
		VX_UNREFERENCED_PARAMETER(sid);

		auto materialIndex = m_editorMaterialCount;

		//writeMaterialToBuffer(pMaterial, m_editorMaterialCount);

		m_editorMaterialIndices.insert(pMaterial, m_editorMaterialCount);

		++m_editorMaterialCount;

		return materialIndex;
	}

	void RenderData::updateMouseHit(const vx::float3 &p)
	{
		auto ptr = m_mouseHitVertex.map<vx::float3>(vx::gl::Map::Write_Only);
		*ptr = p;
	}

	void RenderData::updateWaypoint(u32 offset, u32 count, const Waypoint* src)
	{
		auto offsetBytes = offset * sizeof(Waypoint);
		auto sizeBytes = count * sizeof(Waypoint);

		auto ptr = m_editorWaypointsVbo.mapRange<Waypoint>(offsetBytes, sizeBytes, vx::gl::MapRange::Write);
		::memcpy(ptr.get(), src + offset, sizeBytes);

		++m_waypointCount;
	}

	u8 RenderData::getMeshInstanceId(vx::StringID sid, u32* id)
	{
		auto it = m_editorMeshInstanceIndices.find(sid);

		u8 result = 0;
		if (it != m_editorMeshInstanceIndices.end())
		{
			result = 1;
			*id = *it;
		}

		return result;
	}
}