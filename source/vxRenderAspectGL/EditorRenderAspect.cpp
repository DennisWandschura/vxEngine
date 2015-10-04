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
#include "EditorRenderAspect.h"
#include <vxEngineLib/EngineConfig.h>
#include <vxgl/gl.h>
#include <vxLib/File/FileHandle.h>
#include "Graphics/Renderer.h"
#include "Graphics/CommandListFactory.h"
#include <vxEngineLib/Graphics/TextureFactory.h>
#include <vxEngineLib/Graphics/Texture.h>
#include <vxEngineLib/Message.h>
#include <vxEngineLib/EditorScene.h>
#include <vxEngineLib/FileMessage.h>
#include <vxEngineLib/MessageTypes.h>
#include <vxEngineLib/EditorMeshInstance.h>
#include <vxEngineLib/Spawn.h>
#include <vxEngineLib/InfluenceMap.h>
#include <vxEngineLib/Waypoint.h>
#include <vxEngineLib/NavMeshGraph.h>
#include <UniformCameraBuffer.h>
#include <vxGL/StateManager.h>
#include <EditorLightBuffer.h>
#include "gl/BufferBindingManager.h"
#include <UniformCameraBufferStatic.h>
#include <vxEngineLib/Graphics/Light.h>
#include <vxGL/Debug.h>
#include <vxLib/ScopeGuard.h>
#include <vxGL/VertexArray.h>
#include <vxEngineLib/Material.h>
#include "GpuStructs.h"
#include "Graphics/Commands.h"
#include "Graphics/Segment.h"
#include "Graphics/State.h"
#include <vxGL/ProgramPipeline.h>
#include <vxEngineLib/Joint.h>
#include <vxEngineLib/ArrayAllocator.h>
#include <vxEngineLib/RendererMessage.h>
#include <vxEngineLib/Graphics/LightGeometryProxy.h>
#include <vxEngineLib/Plane.h>
#include <vxGL/Framebuffer.h>
#include <vxEngineLib/algorithm.h>

struct InfluenceCellVertex
{
	vx::float3 position;
	u32 count;
};

namespace EditorRenderAspectCpp
{
	auto createPlanes = [](const vx::float3 &vmin, const vx::float3 &vmax, Plane* planes)
	{
		vx::float3 nbl = vx::float3(vmin.x, vmin.y, vmax.z);
		vx::float3 nbr = vx::float3(vmax.x, vmin.y, vmax.z);

		vx::float3 ntl = vx::float3(vmin.x, vmax.y, vmax.z);
		vx::float3 ntr = vx::float3(vmax.x, vmax.y, vmax.z);

		vx::float3 fbl = vx::float3(vmin.x, vmin.y, vmin.z);
		vx::float3 fbr = vx::float3(vmax.x, vmin.y, vmin.z);

		vx::float3 ftl = vx::float3(vmin.x, vmax.y, vmin.z);
		vx::float3 ftr = vx::float3(vmax.x, vmax.y, vmin.z);

		// x+, x-, y+, y-, z+, z-
		planes[0] = Plane::create(nbl, fbl, ftl);
		planes[1] = Plane::create(fbr, nbr, ntr);
		planes[2] = Plane::create(nbl, nbr, fbr);
		planes[3] = Plane::create(ftl, ftr, ntr);

		planes[4] = Plane::create(fbl, fbr, ftr);
		planes[5] = Plane::create(nbr, nbl, ntl);
	};

	auto intersectAABBPlane = [](const AABB &bounds, const Plane &plane)
	{
		auto c = (bounds.max + bounds.min) * 0.5f;
		auto e = bounds.max - c;

		auto r = e.x * abs(plane.n.x) + e.y * abs(plane.n.y) + e.z * abs(plane.n.z);
		auto s = vx::dot3(plane.n, c) - plane.d;

		if ((fabsf(s) + 0.0001f) <= r)
		{
			return true;
		}
		return false;
	};

	void generateProxyGeometry(const Graphics::LightGeometryProxy* proxies, u32 proxyCount, std::vector<AABB>* geometry, std::vector<s32>* masks, std::vector<std::pair<u32, u32>>* tmp)
	{
		auto testBounds = [&](const AABB &bounds, const Plane(&planes)[6], std::vector<AABB>* outNewBounds, std::vector<s32>* masks, AABB* intersectionBounds, s32* intersectionMask, const std::pair<u32, u32> &pair, std::vector<std::pair<u32, u32>>* tmp)
		{
			const Plane* intersectingPlanes[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
			bool intersect = false;
			for (u32 k = 0; k < 6; ++k)
			{
				if (intersectAABBPlane(bounds, planes[k]))
				{
					intersectingPlanes[k] = &planes[k];
					intersect = true;
				}
			}

			for (u32 k = 0; k < 6; ++k)
			{
				auto intersectingPlane = intersectingPlanes[k];
				if (intersectingPlane)
				{
					vx::float3 p = intersectingPlane->n * intersectingPlane->d;

					if (p.x != 0.0f)
					{
						s32 mask = 0;
						AABB newBounds = bounds;
						if (intersectingPlane->n.x < 0.0f)
						{
							mask |= 1 << 0;
							newBounds.min.x = p.x;

							intersectionBounds->max.x = p.x;
							*intersectionMask |= 1 << 1;
						}
						else
						{
							mask |= 1 << 1;
							newBounds.max.x = p.x;

							intersectionBounds->min.x = p.x;
							*intersectionMask |= 1 << 0;
						}
						outNewBounds->push_back(newBounds);
						masks->push_back(mask);
						tmp->push_back(pair);
					}

					if (p.y != 0.0f)
					{
						s32 mask = 0;
						AABB newBounds = bounds;
						if (intersectingPlane->n.y < 0.0f)
						{
							mask |= 1 << 2;
							newBounds.min.y = p.y;

							intersectionBounds->max.y = p.y;
							*intersectionMask |= 1 << 3;
						}
						else
						{
							mask |= 1 << 3;
							newBounds.max.y = p.y;

							intersectionBounds->min.y = p.y;
							*intersectionMask |= 1 << 2;
						}
						outNewBounds->push_back(newBounds);
						masks->push_back(mask);
						tmp->push_back(pair);
					}

					if (p.z != 0.0f)
					{
						s32 mask = 0;
						AABB newBounds = bounds;
						if (intersectingPlane->n.z < 0.0f)
						{
							mask |= 1 << 5;
							newBounds.min.z = p.z;

							intersectionBounds->max.z = p.z;
							*intersectionMask |= 1 << 4;
						}
						else
						{
							mask |= 1 << 4;
							newBounds.max.z = p.z;

							intersectionBounds->min.z = p.z;
							*intersectionMask |= 1 << 5;
						}
						outNewBounds->push_back(newBounds);
						masks->push_back(mask);
						tmp->push_back(pair);
					}
				}
			}

			return intersect;
		};

		auto CmpPair = [](const std::pair<u32, u32> &lhs, const std::pair<u32, u32> &rhs)
		{
			if (lhs.first < rhs.first)
				return true;
			else if (lhs.first == rhs.first)
			{
				return lhs.second < rhs.second;
			}

			return false;
		};

		if (proxyCount != 0)
		{
			std::vector<std::pair<u32, u32>> pairsToTest;
			for (u32 i = 0; i < proxyCount; ++i)
			{
				for (u32 j = 0; j < proxyCount; ++j)
				{
					if (i != j)
					{
						auto pair = std::make_pair(std::min(i, j), std::max(i, j));

						auto it = vx::vector_find(pairsToTest, pair, CmpPair);
						if (it == pairsToTest.end())
						{
							vx::vector_sortedInsert(&pairsToTest, pair, CmpPair);
						}
					}
				}
			}

			std::vector<u32> overlappingIndices;
			std::vector<u32> remainingIndices;
			for (auto &it : pairsToTest)
			{
				auto i0 = it.first;
				auto i1 = it.second;

				auto &bounds0 = proxies[i0].m_bounds;
				auto &bounds1 = proxies[i1].m_bounds;

				if (bounds0.intersects(bounds1))
				{
					Plane planes0[6];
					createPlanes(bounds0.min, bounds0.max, planes0);

					Plane planes1[6];
					createPlanes(bounds1.min, bounds1.max, planes1);

					AABB intersectionBounds = bounds0;
					s32 intersectionMask = 0;
					testBounds(bounds0, planes1, geometry, masks, &intersectionBounds, &intersectionMask, it, tmp);

					testBounds(bounds1, planes0, geometry, masks, &intersectionBounds, &intersectionMask, it, tmp);

					geometry->push_back(intersectionBounds);
					masks->push_back(intersectionMask);
					tmp->push_back(it);

					auto iter = vx::vector_find(overlappingIndices, i0, std::less<u32>());
					if (iter == overlappingIndices.end())
					{
						vx::vector_sortedInsert(&overlappingIndices, i0, std::less<u32>());
					}

					iter = vx::vector_find(overlappingIndices, i1, std::less<u32>());
					if (iter == overlappingIndices.end())
					{
						vx::vector_sortedInsert(&overlappingIndices, i1, std::less<u32>());
					}
				}
				else
				{
					auto iter0 = vx::vector_find(overlappingIndices, i0, std::less<u32>());
					auto iter1 = vx::vector_find(overlappingIndices, i1, std::less<u32>());

					auto iter = vx::vector_find(remainingIndices, i0, std::less<u32>());
					if (iter == remainingIndices.end() && iter0 == overlappingIndices.end())
					{
						vx::vector_sortedInsert(&remainingIndices, i0, std::less<u32>());
					}

					iter = vx::vector_find(remainingIndices, i1, std::less<u32>());
					if (iter == remainingIndices.end() && iter1 == overlappingIndices.end())
					{
						vx::vector_sortedInsert(&remainingIndices, i1, std::less<u32>());
					}
					remainingIndices;
				}
			}

			for (auto &it : remainingIndices)
			{
				auto iter = vx::vector_find(overlappingIndices, it, std::less<u32>());
				if (iter == overlappingIndices.end())
				{
					auto &bounds = proxies[it].m_bounds;
					geometry->push_back(bounds);
					masks->push_back(0);

					tmp->push_back(std::make_pair(it, it));
				}
			}
		}
	}
}

namespace Editor
{
	//enum class RenderAspect::EditorUpdate : u32{ Update_None, Update_Mesh, Update_Material, Editor_Added_Instance, Editor_Update_Instance, Editor_Set_Scene };

	void __stdcall renderDebugCallback()
	{
		std::abort();
	}

	struct VoxelData
	{
		u32 dim;
		u32 halfDim;
		f32 gridCellSize;
		f32 invGridCellSize;
		f32 gridCellSizeY;
		f32 invGridCellSizeY;
	};

	struct JointData
	{
		vx::float4 p0;
		vx::float4 p1;
	};

	const u32 g_maxLightCount{ 128 };

	struct VertexPositionColor
	{
		vx::float3 position;
		vx::float3 color;
	};

	struct LightGeometryProxyData
	{
		vx::float4 vmin;
		vx::float4 vmax;
	};

	struct RenderAspect::ColdData
	{
		vx::gl::Texture m_texture;
		u32 m_navmeshConnectionIndexCount;
		u32 m_influenceMapIndexCount;
	};

	RenderAspect::RenderAspect()
		:m_resourceAspect(nullptr)
	{

	}

	RenderAspect::~RenderAspect()
	{

	}

	RenderAspectInitializeError RenderAspect::initializeImpl(const RenderAspectDescription &renderDesc)
	{
		auto resolution = renderDesc.settings->m_resolution;
		f32 znear = renderDesc.settings->m_zNear;
		f32 zfar = renderDesc.settings->m_zFar;
		m_resolution = resolution;
		m_projectionMatrix = vx::MatrixPerspectiveFovRHDX(vx::degToRad(renderDesc.settings->m_fovDeg), (f32)resolution.x / (f32)resolution.y, znear, zfar);

		m_resourceAspect = renderDesc.resourceAspect;

		vx::gl::ContextDescription contextDesc;
		contextDesc.tmpHwnd = (HWND)renderDesc.tmpHwnd;
		contextDesc.glParams.hwnd = (HWND)renderDesc.hwnd;
		contextDesc.glParams.resolution = resolution;
		contextDesc.glParams.majVersion = 4;
		contextDesc.glParams.minVersion = 5;
		contextDesc.glParams.bVsync = 0;
		contextDesc.glParams.bDebugMode = 1;

		if (!m_renderContext.initialize(contextDesc))
		{
			return RenderAspectInitializeError::ERROR_CONTEXT;
		}

		vx::gl::Debug::initialize();
		vx::gl::Debug::setHighSeverityCallback(renderDebugCallback);
		vx::gl::Debug::enableCallback(true);

		const auto memorySize = 10 MBYTE;
		auto memory = renderDesc.pAllocator->allocate(memorySize, 16);
		if (memory == nullptr)
		{
			return RenderAspectInitializeError::ERROR_OUT_OF_MEMORY;
		}

		m_allocator = vx::StackAllocator(memory, memorySize);

		const auto scratchMemorySize = 5 MBYTE;
		auto scratchMemory = renderDesc.pAllocator->allocate(scratchMemorySize, 16);
		if (scratchMemory == nullptr)
		{
			return RenderAspectInitializeError::ERROR_OUT_OF_MEMORY;
		}
		m_scratchAllocator = vx::StackAllocator(scratchMemory, scratchMemorySize);

		m_coldData = vx::make_unique<ColdData>();
		m_coldData->m_navmeshConnectionIndexCount = 0;
		m_coldData->m_influenceMapIndexCount = 0;

		vx::gl::StateManager::disable(vx::gl::Capabilities::Framebuffer_sRGB);
		vx::gl::StateManager::enable(vx::gl::Capabilities::Texture_Cube_Map_Seamless);
		vx::gl::StateManager::setClearColor(0, 0, 0, 1);
		vx::gl::StateManager::setViewport(0, 0, m_resolution.x, m_resolution.y);

		m_objectManager.initialize(50, 20, 20, 20, &m_allocator);
		m_camera.setPosition(0, 5, 5);

		m_shaderManager.initialize(renderDesc.dataDir);

		m_objectManager.createVertexArray("emptyVao");

		{
			vx::gl::BufferDescription cameraDesc;
			cameraDesc.bufferType = vx::gl::BufferType::Uniform_Buffer;
			cameraDesc.size = sizeof(UniformCameraBufferBlock);
			cameraDesc.immutable = 1;
			cameraDesc.flags = vx::gl::BufferStorageFlags::Write | vx::gl::BufferStorageFlags::Dynamic_Storage;
			m_cameraBuffer.create(cameraDesc);
		}

		{
			UniformCameraBufferStaticBlock cameraBlockStatic;
			cameraBlockStatic.invProjectionMatrix = vx::MatrixInverse(m_projectionMatrix);
			cameraBlockStatic.projectionMatrix = m_projectionMatrix;
			cameraBlockStatic.orthoMatrix = vx::MatrixOrthographicRHDX(m_resolution.x, m_resolution.y, znear, zfar);

			vx::gl::BufferDescription cameraDesc;
			cameraDesc.bufferType = vx::gl::BufferType::Uniform_Buffer;
			cameraDesc.immutable = 1;
			cameraDesc.size = sizeof(UniformCameraBufferStaticBlock);
			cameraDesc.flags = vx::gl::BufferStorageFlags::None;
			cameraDesc.pData = &cameraBlockStatic;

			m_objectManager.createBuffer("CameraBufferStatic", cameraDesc);
		}

		{
			vx::gl::BufferDescription desc;
			desc.bufferType = vx::gl::BufferType::Shader_Storage_Buffer;
			desc.immutable = 1;
			desc.size = sizeof(LightData) * g_maxLightCount + sizeof(vx::uint4);
			desc.flags = vx::gl::BufferStorageFlags::Write;
			desc.pData = nullptr;

			m_objectManager.createBuffer("editorLightBuffer", desc);
		}

		{
			Gpu::UniformTextureBufferBlock data;
			//data.u_aabbTexture = m_pColdData->m_aabbTexture.getTextureHandle();
			//data.u_ambientSlice = m_pColdData->m_ambientColorTexture.getTextureHandle();
			//data.u_ambientImage = m_pColdData->m_ambientColorTexture.getImageHandle(0, 0, 0);

			vx::gl::BufferDescription desc;
			desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
			desc.size = sizeof(Gpu::UniformTextureBufferBlock);
			desc.immutable = 1;
			desc.pData = &data;
			desc.flags = vx::gl::BufferStorageFlags::Write;

			m_objectManager.createBuffer("UniformTextureBuffer", desc);
		}

		m_shaderManager.setDefine("NOSHADOWS");
		m_shaderManager.addParameter("maxShadowLights", 5);

		auto shaderIncludeDir = renderDesc.dataDir + "shaders/include/";
		m_shaderManager.addIncludeFile((shaderIncludeDir + "structs.glsl").c_str(), "structs.glsl");
		m_shaderManager.addIncludeFile((shaderIncludeDir + "math.glsl").c_str(), "math.glsl");
		m_shaderManager.addIncludeFile((shaderIncludeDir + "buffers.glsl").c_str(), "buffers.glsl");
		m_shaderManager.addIncludeFile((shaderIncludeDir + "uniform_buffers.glsl").c_str(), "uniform_buffers.glsl");
		m_shaderManager.addIncludeFile((shaderIncludeDir + "mesh.glsl").c_str(), "mesh.glsl");
		m_shaderManager.addIncludeFile((shaderIncludeDir + "common.h").c_str(), "common.h");
		m_shaderManager.addIncludeFile((shaderIncludeDir + "UniformCameraBuffer.h").c_str(), "UniformCameraBuffer.h");
		m_shaderManager.addIncludeFile((shaderIncludeDir + "UniformShadowTextureBuffer.h").c_str(), "UniformShadowTextureBuffer.h");
		m_shaderManager.addIncludeFile((shaderIncludeDir + "UniformShadowTransformBuffer.h").c_str(), "UniformShadowTransformBuffer.h");
		m_shaderManager.addIncludeFile((shaderIncludeDir + "UniformCameraBufferStatic.h").c_str(), "UniformCameraBufferStatic.h");
		m_shaderManager.addIncludeFile((shaderIncludeDir + "EditorLightBuffer.h").c_str(), "EditorLightBuffer.h");

		std::string error;
		m_shaderManager.loadPipeline(vx::FileHandle("editorDrawSpawn.pipe"), "editorDrawSpawn.pipe", &m_allocator, &error);
		m_shaderManager.loadPipeline(vx::FileHandle("editorDrawPointColor.pipe"), "editorDrawPointColor.pipe", &m_allocator, &error);
		m_shaderManager.loadPipeline(vx::FileHandle("editorDrawPoint.pipe"), "editorDrawPoint.pipe", &m_allocator, &error);
		m_shaderManager.loadPipeline(vx::FileHandle("editorDrawInfluenceCell.pipe"), "editorDrawInfluenceCell.pipe", &m_allocator, &error);
		m_shaderManager.loadPipeline(vx::FileHandle("editorDrawLights.pipe"), "editorDrawLights.pipe", &m_allocator, &error);
		m_shaderManager.loadPipeline(vx::FileHandle("editorDrawMesh.pipe"), "editorDrawMesh.pipe", &m_allocator, &error);
		m_shaderManager.loadPipeline(vx::FileHandle("navmesh.pipe"), "navmesh.pipe", &m_allocator, &error);
		m_shaderManager.loadPipeline(vx::FileHandle("editorDrawNavmeshConnection.pipe"), "editorDrawNavmeshConnection.pipe", &m_allocator, &error);
		m_shaderManager.loadPipeline(vx::FileHandle("editorSelectedMesh.pipe"), "editorSelectedMesh.pipe", &m_allocator, &error);
		m_shaderManager.loadPipeline(vx::FileHandle("editorDrawVoxelGrid.pipe"), "editorDrawVoxelGrid.pipe", &m_allocator, &error);
		m_shaderManager.loadPipeline(vx::FileHandle("editorDrawJoints.pipe"), "editorDrawJoints.pipe", &m_allocator, &error);
		m_shaderManager.loadPipeline(vx::FileHandle("editorDrawLightGeometryProxy.pipe"), "editorDrawLightGeometryProxy.pipe", &m_allocator, &error);
		m_shaderManager.loadPipeline(vx::FileHandle("editorTestLightGeometryProxy.pipe"), "editorTestLightGeometryProxy.pipe", &m_allocator, &error);
		m_shaderManager.loadPipeline(vx::FileHandle("editorTestLightGeometryProxy0.pipe"), "editorTestLightGeometryProxy0.pipe", &m_allocator, &error);

		Graphics::Renderer::provide(&m_shaderManager, &m_objectManager, renderDesc.settings, nullptr);

		auto maxInstances = renderDesc.settings->m_rendererSettings.m_maxMeshInstances;

		m_materialManager.initialize(vx::uint3(1024, 1024, 128), maxInstances, &m_objectManager);
		m_meshManager.initialize(maxInstances, 0xffff + 1, 0xffff + 1, &m_objectManager);

		{
			vx::gl::BufferDescription navmeshVertexVboDesc;
			navmeshVertexVboDesc.bufferType = vx::gl::BufferType::Array_Buffer;
			navmeshVertexVboDesc.flags = vx::gl::BufferStorageFlags::Write;
			navmeshVertexVboDesc.immutable = 1;
			navmeshVertexVboDesc.pData = nullptr;
			navmeshVertexVboDesc.size = sizeof(VertexPositionColor) * 256;
			m_objectManager.createBuffer("navMeshVertexVbo", navmeshVertexVboDesc);
		}

		{
			vx::gl::BufferDescription navMeshVertexIboDesc;
			navMeshVertexIboDesc.bufferType = vx::gl::BufferType::Element_Array_Buffer;
			navMeshVertexIboDesc.flags = vx::gl::BufferStorageFlags::Write;
			navMeshVertexIboDesc.immutable = 1;
			navMeshVertexIboDesc.pData = nullptr;
			navMeshVertexIboDesc.size = sizeof(u16) * 256 * 3;
			m_objectManager.createBuffer("navMeshVertexIbo", navMeshVertexIboDesc);
		}

		createNavMeshVertexVao();
		createNavMeshVao();

		{
			vx::gl::BufferDescription desc;
			desc.bufferType = vx::gl::BufferType::Array_Buffer;
			desc.flags = vx::gl::BufferStorageFlags::Write;
			desc.immutable = 1;
			desc.pData = nullptr;
			desc.size = sizeof(vx::float3) * 256;
			m_objectManager.createBuffer("navMeshGraphNodesVbo", desc);
		}

		createNavMeshNodesVao();


		{
			vx::gl::BufferDescription desc;
			desc.bufferType = vx::gl::BufferType::Array_Buffer;
			desc.flags = vx::gl::BufferStorageFlags::Write;
			desc.immutable = 1;
			desc.pData = nullptr;
			desc.size = sizeof(vx::float3) * 256;
			m_objectManager.createBuffer("spawnPointVbo", desc);
		}

		auto spawnPointVbo = m_objectManager.getBuffer("spawnPointVbo");

		m_objectManager.createVertexArray("spawnPointVao");
		auto m_spawnPointVao = m_objectManager.getVertexArray("spawnPointVao");

		m_spawnPointVao->create();
		m_spawnPointVao->enableArrayAttrib(0);
		m_spawnPointVao->arrayAttribBinding(0, 0);
		m_spawnPointVao->arrayAttribFormatF(0, 3, 0, 0);
		m_spawnPointVao->bindVertexBuffer(*spawnPointVbo, 0, 0, sizeof(vx::float3));

		createIndirectCmdBuffers();

		{
			auto vaoSid = m_objectManager.createVertexArray("drawInfluenceCellNewVao");

			vx::gl::BufferDescription desc;
			desc.bufferType = vx::gl::BufferType::Array_Buffer;
			desc.flags = vx::gl::BufferStorageFlags::Write;
			desc.immutable = 1;
			desc.pData = nullptr;
			desc.size = sizeof(VertexPositionColor) * 256 * 3;

			auto vboSid = m_objectManager.createBuffer("drawInfluenceCellNewVbo", desc);

			vx::gl::DrawArraysIndirectCommand cmd;
			cmd.baseInstance = 0;
			cmd.count = 0;
			cmd.first = 0;
			cmd.instanceCount = 1;

			desc.bufferType = vx::gl::BufferType::Draw_Indirect_Buffer;
			desc.pData = &cmd;
			desc.size = sizeof(vx::gl::DrawArraysIndirectCommand);
			m_objectManager.createBuffer("drawInfluenceCellNewCmd", desc);

			auto vao = m_objectManager.getVertexArray(vaoSid);
			auto vbo = m_objectManager.getBuffer(vboSid);
			auto ibo = m_objectManager.getBuffer("navMeshVertexIbo");

			vao->enableArrayAttrib(0);
			vao->enableArrayAttrib(1);

			vao->arrayAttribFormatF(0, 3, 0, 0);
			vao->arrayAttribFormatF(1, 3, 0, sizeof(vx::float3));

			vao->arrayAttribBinding(0, 0);
			vao->arrayAttribBinding(1, 0);

			vao->bindVertexBuffer(*vbo, 0, 0, sizeof(VertexPositionColor));
			vao->bindIndexBuffer(*ibo);
		}

		{
			vx::gl::DrawArraysIndirectCommand cmd = {};
			cmd.instanceCount = 1;

			vx::gl::BufferDescription desc;
			desc.bufferType = vx::gl::BufferType::Draw_Indirect_Buffer;
			desc.flags = vx::gl::BufferStorageFlags::Write | vx::gl::BufferStorageFlags::Dynamic_Storage;
			desc.immutable = 1;
			desc.pData = &cmd;
			desc.size = sizeof(vx::gl::DrawArraysIndirectCommand);

			m_objectManager.createBuffer("waypointCmdBuffer", desc);
		}

		{
			vx::gl::BufferDescription desc;
			desc.bufferType = vx::gl::BufferType::Array_Buffer;
			desc.flags = vx::gl::BufferStorageFlags::Write;
			desc.immutable = 1;
			desc.pData = nullptr;
			desc.size = sizeof(VertexPositionColor) * 256;

			m_objectManager.createBuffer("waypointVbo", desc);
		}

		{
			auto vaoSid = m_objectManager.createVertexArray("waypointVao");

			auto vao = m_objectManager.getVertexArray(vaoSid);
			auto vbo = m_objectManager.getBuffer("waypointVbo");

			vao->enableArrayAttrib(0);
			vao->enableArrayAttrib(1);

			vao->arrayAttribFormatF(0, 3, 0, 0);
			vao->arrayAttribFormatF(1, 3, 0, sizeof(vx::float3));

			vao->arrayAttribBinding(0, 0);
			vao->arrayAttribBinding(1, 0);

			vao->bindVertexBuffer(*vbo, 0, 0, sizeof(VertexPositionColor));
		}

		{
			auto gridSize = renderDesc.settings->m_rendererSettings.m_voxelSettings.m_voxelGridDim;
			auto dim = renderDesc.settings->m_rendererSettings.m_voxelSettings.m_voxelTextureSize;
			f32 gridSizeY = 4.0f;
			f32 gridHalfSize = gridSize / 2.0f;
			f32 gridHalfSizeY = gridSizeY / 2.0f;

			VoxelData data;
			data.dim = dim;
			data.halfDim = data.dim / 2;
			data.gridCellSize = gridHalfSize / data.halfDim;
			data.invGridCellSize = 1.0f / data.gridCellSize;
			data.gridCellSizeY = gridHalfSizeY / data.halfDim;
			data.invGridCellSizeY = 1.0f / data.gridCellSizeY;

			vx::gl::BufferDescription desc;
			desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
			desc.flags = vx::gl::BufferStorageFlags::Write;
			desc.immutable = 1;
			desc.pData = &data;
			desc.size = sizeof(VoxelData);

			m_objectManager.createBuffer("editorVoxelDataBuffer", desc);
		}

		{
			vx::gl::BufferDescription desc;
			desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
			desc.flags = vx::gl::BufferStorageFlags::Write;
			desc.immutable = 1;
			desc.pData = nullptr;
			desc.size = sizeof(JointData) * 256;

			m_objectManager.createBuffer("editorJointBuffer", desc);

			vx::gl::DrawArraysIndirectCommand cmd{};
			cmd.instanceCount = 1;

			desc.bufferType = vx::gl::BufferType::Draw_Indirect_Buffer;
			desc.pData = &cmd;
			desc.size = sizeof(cmd);
			m_objectManager.createBuffer("editorJointCmdBuffer", desc);
		}

		{
			vx::gl::BufferDescription bufferDesc;
			bufferDesc.bufferType = vx::gl::BufferType::Uniform_Buffer;
			bufferDesc.flags = vx::gl::BufferStorageFlags::Write;
			bufferDesc.immutable = 1;
			bufferDesc.pData = nullptr;
			bufferDesc.size = sizeof(LightGeometryProxyData) * 256;

			m_objectManager.createBuffer("editorLightGeometryProxyBuffer", bufferDesc);

			vx::gl::DrawArraysIndirectCommand cmd{};
			cmd.instanceCount = 1;
			cmd.count = 0;

			bufferDesc.bufferType = vx::gl::BufferType::Draw_Indirect_Buffer;
			bufferDesc.pData = &cmd;
			bufferDesc.size = sizeof(cmd);
			bufferDesc.flags = vx::gl::BufferStorageFlags::Write | vx::gl::BufferStorageFlags::Dynamic_Storage;
			m_objectManager.createBuffer("editorLightGeometryProxyCmdBuffer", bufferDesc);
		}

		m_commandList.initialize();
		createCommandList();

		if (!createEditorTextures())
		{
			puts("Error loading editor textures !");
			return RenderAspectInitializeError::ERROR_CONTEXT;
		}

		bindBuffers();

		glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		updateCamera();

		return RenderAspectInitializeError::OK;
	}

	void RenderAspect::bindBuffers()
	{
		auto editorTextureBuffer = m_objectManager.getBuffer("editorTextureBuffer");
		auto editorLightBuffer = m_objectManager.getBuffer("editorLightBuffer");

		auto pUniformTextureBuffer = m_objectManager.getBuffer("UniformTextureBuffer");
		auto pCameraBufferStatic = m_objectManager.getBuffer("CameraBufferStatic");
		auto transformBuffer = m_objectManager.getBuffer("transformBuffer");
		auto materialBlockBuffer = m_objectManager.getBuffer("materialBlockBuffer");
		auto editorVoxelDataBuffer = m_objectManager.getBuffer("editorVoxelDataBuffer");
		auto editorJointBuffer = m_objectManager.getBuffer("editorJointBuffer");
		auto editorLightGeometryProxyBuffer = m_objectManager.getBuffer("editorLightGeometryProxyBuffer");

		gl::BufferBindingManager::bindBaseUniform(0, m_cameraBuffer.getId());
		gl::BufferBindingManager::bindBaseUniform(2, pCameraBufferStatic->getId());
		gl::BufferBindingManager::bindBaseUniform(3, pUniformTextureBuffer->getId());
		gl::BufferBindingManager::bindBaseUniform(4, editorVoxelDataBuffer->getId());

		gl::BufferBindingManager::bindBaseUniform(8, editorTextureBuffer->getId());
		gl::BufferBindingManager::bindBaseUniform(9, editorJointBuffer->getId());
		gl::BufferBindingManager::bindBaseUniform(10, editorLightGeometryProxyBuffer->getId());

		gl::BufferBindingManager::bindBaseShaderStorage(0, transformBuffer->getId());
		gl::BufferBindingManager::bindBaseShaderStorage(1, materialBlockBuffer->getId());
		//gl::BufferBindingManager::bindBaseShaderStorage(2, pTextureBuffer->getId());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, editorLightBuffer->getId());
	}

	bool RenderAspect::createEditorTextures()
	{
		auto textureDim = vx::ushort3(512, 512, 2);

		vx::gl::TextureDescription desc;
		desc.format = vx::gl::TextureFormat::SRGBA_DXT5;
		desc.miplevels = 1;
		desc.size = textureDim;
		desc.sparse = 0;
		desc.type = vx::gl::TextureType::Texture_2D_Array;

		m_coldData->m_texture.create(desc);

		Graphics::Texture ddsFileLight;
		Graphics::Texture ddsFileSpawn;

		auto allocMarker = m_allocator.getMarker();
		SCOPE_EXIT
		{
			m_allocator.clear(allocMarker);
		};

		ArrayAllocator tmpAllocator;
		tmpAllocator.create(m_allocator.allocate(5 MBYTE), 5 MBYTE);

		if (!Graphics::TextureFactory::createDDSFromFile("../../data/textures/editor/light.dds", true, true, &ddsFileLight, &tmpAllocator, &m_scratchAllocator))
		{
			puts("could not create texture light.dds");
			return false;
		}

		if (!Graphics::TextureFactory::createDDSFromFile("../../data/textures/editor/spawnPoint.dds", true, true, &ddsFileSpawn, &tmpAllocator, &m_scratchAllocator))
		{
			puts("could not create texture spawnPoint.dds");
			return false;
		}

		/*if (!ddsFileLight.loadFromFile("../../data/textures/editor/light.dds") ||
			!ddsFileSpawn.loadFromFile("../../data/textures/editor/spawnPoint.dds"))
		{
			puts("Error loading texture !");
			VX_ASSERT(false);
			return;
		}

		auto &textureLight = ddsFileLight.getTexture(0);
		auto &textureSpawn = ddsFileSpawn.getTexture(0);*/

		auto &textureLight = ddsFileLight.getFace(0);
		auto &textureSpawn = ddsFileSpawn.getFace(0);

		auto lightTexDim = textureLight.getDimension();
		auto spawnTexDim = textureSpawn.getDimension();

		if (textureDim.x != lightTexDim.x)
			return false;

		if (textureDim.x != spawnTexDim.x)
			return false;

		vx::gl::TextureCompressedSubImageDescription subImgDesc;
		subImgDesc.dataSize = textureLight.getSize();
		subImgDesc.miplevel = 0;
		subImgDesc.offset = vx::ushort3(0, 0, 0);
		subImgDesc.p = textureLight.getPixels();
		subImgDesc.size = vx::ushort3(textureDim.x, textureDim.y, 1);
		m_coldData->m_texture.subImageCompressed(subImgDesc);

		subImgDesc.offset = vx::ushort3(0, 0, 1);
		subImgDesc.dataSize = textureSpawn.getSize();
		subImgDesc.p = textureSpawn.getPixels();
		m_coldData->m_texture.subImageCompressed(subImgDesc);

		auto handle = m_coldData->m_texture.getTextureHandle();
		m_coldData->m_texture.makeTextureResident();

		vx::gl::BufferDescription bufferDesc;
		bufferDesc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		bufferDesc.flags = 0;
		bufferDesc.immutable = 1;
		bufferDesc.pData = &handle;
		bufferDesc.size = sizeof(u64);

		m_objectManager.createBuffer("editorTextureBuffer", bufferDesc);

		ddsFileLight.clear();
		ddsFileSpawn.clear();

		return true;
	}

	void RenderAspect::createNavMeshNodesVao()
	{
		m_objectManager.createVertexArray("navMeshGraphNodesVao");
		auto navMeshGraphNodesVao = m_objectManager.getVertexArray("navMeshGraphNodesVao");
		navMeshGraphNodesVao->create();

		navMeshGraphNodesVao->enableArrayAttrib(0);
		navMeshGraphNodesVao->arrayAttribFormatF(0, 3, 0, 0);
		navMeshGraphNodesVao->arrayAttribBinding(0, 0);

		auto navMeshGraphNodesVbo = m_objectManager.getBuffer("navMeshGraphNodesVbo");
		navMeshGraphNodesVao->bindVertexBuffer(*navMeshGraphNodesVbo, 0, 0, sizeof(vx::float3));
	}

	void RenderAspect::createIndirectCmdBuffers()
	{
		{
			vx::gl::DrawArraysIndirectCommand arrayCmd = {};
			arrayCmd.instanceCount = 1;

			vx::gl::BufferDescription desc;
			desc.bufferType = vx::gl::BufferType::Draw_Indirect_Buffer;
			desc.flags = vx::gl::BufferStorageFlags::Write;
			desc.immutable = 1;
			desc.pData = &arrayCmd;
			desc.size = sizeof(vx::gl::DrawArraysIndirectCommand);

			m_objectManager.createBuffer("editorLightCmdBuffer", desc);
			m_objectManager.createBuffer("navMeshVertexCmdBuffer", desc);
			m_objectManager.createBuffer("graphNodesCmdBuffer", desc);
			m_objectManager.createBuffer("spawnPointCmdBuffer", desc);
		}

		{
			vx::gl::DrawElementsIndirectCommand elementsCmd;
			memset(&elementsCmd, 0, sizeof(vx::gl::DrawElementsIndirectCommand));
			elementsCmd.instanceCount = 1;

			vx::gl::BufferDescription desc;
			desc.bufferType = vx::gl::BufferType::Draw_Indirect_Buffer;
			desc.flags = vx::gl::BufferStorageFlags::Write;
			desc.immutable = 1;
			desc.pData = &elementsCmd;
			desc.size = sizeof(vx::gl::DrawElementsIndirectCommand);

			m_objectManager.createBuffer("navmeshCmdBuffer", desc);
		}
	}

	void RenderAspect::createNavMeshVao()
	{
		auto navMeshVertexVbo = m_objectManager.getBuffer("navMeshVertexVbo");
		auto ibo = m_objectManager.getBuffer("navMeshVertexIbo");

		m_objectManager.createVertexArray("navMeshVao");
		auto navMeshVao = m_objectManager.getVertexArray("navMeshVao");
		navMeshVao->enableArrayAttrib(0);
		navMeshVao->arrayAttribFormatF(0, 3, 0, 0);
		navMeshVao->arrayAttribBinding(0, 0);
		navMeshVao->bindVertexBuffer(*navMeshVertexVbo, 0, 0, sizeof(VertexPositionColor));

		navMeshVao->bindIndexBuffer(*ibo);
	}

	void RenderAspect::createNavMeshVertexVao()
	{
		auto navMeshVertexVbo = m_objectManager.getBuffer("navMeshVertexVbo");

		m_objectManager.createVertexArray("navMeshVertexVao");
		auto navMeshVertexVao = m_objectManager.getVertexArray("navMeshVertexVao");

		navMeshVertexVao->create();
		navMeshVertexVao->enableArrayAttrib(0);
		navMeshVertexVao->arrayAttribFormatF(0, 3, 0, 0);
		navMeshVertexVao->arrayAttribBinding(0, 0);

		navMeshVertexVao->enableArrayAttrib(1);
		navMeshVertexVao->arrayAttribFormatF(1, 3, 0, sizeof(vx::float3));
		navMeshVertexVao->arrayAttribBinding(1, 0);

		navMeshVertexVao->bindVertexBuffer(*navMeshVertexVbo, 0, 0, sizeof(VertexPositionColor));
	}

	void RenderAspect::createVoxelSegment()
	{
		auto emptyVao = m_objectManager.getVertexArray("emptyVao");
		auto pipeline = m_shaderManager.getPipeline("editorDrawVoxelGrid.pipe");

		Graphics::StateDescription stateDesc
		{
			0,
			emptyVao->getId(),
			pipeline->getId(),
			0,
			0,
			false,
			true,
			false,
			true,
			{1, 1, 1, 1},
			1
		};

		Graphics::State state;
		state.set(stateDesc);

		u32 voxelDim = 128;

		Graphics::DrawArraysInstancedCommand drawCmd;
		drawCmd.set(GL_POINTS, 0, voxelDim, voxelDim * voxelDim);

		//Graphics::BlendEquationCommand blendCmd;
		//blendCmd.set();

		Graphics::Segment segment;
		segment.setState(state);
		//segment.pushCommand(drawCmd);

		//m_commandList.pushSegment(segment, "editorDrawVoxelGrid");
	}

	void RenderAspect::createCommandList()
	{
		vx::gl::BufferDescription bufferDesc;
		bufferDesc.bufferType = vx::gl::BufferType::Array_Buffer;
		bufferDesc.flags = vx::gl::BufferStorageFlags::Write;
		bufferDesc.immutable = 1;
		bufferDesc.size = sizeof(vx::float3) * 2 * 1024;

		auto bufferSid = m_objectManager.createBuffer("editorDrawNavmeshConnectionVbo", bufferDesc);

		vx::gl::DrawArraysIndirectCommand cmd;
		cmd.baseInstance = 0;
		cmd.count = 0;
		cmd.first = 0;
		cmd.instanceCount = 1;

		bufferDesc.bufferType = vx::gl::BufferType::Draw_Indirect_Buffer;
		bufferDesc.size = sizeof(vx::gl::DrawArraysIndirectCommand);
		bufferDesc.pData = &cmd;
		bufferDesc.flags = vx::gl::BufferStorageFlags::Write;
		auto cmdSid = m_objectManager.createBuffer("editorDrawNavmeshConnectionCmd", bufferDesc);
		auto vaoSid = m_objectManager.createVertexArray("editorDrawNavmeshConnectionVao");

		auto vao = m_objectManager.getVertexArray(vaoSid);
		auto vbo = m_objectManager.getBuffer(bufferSid);

		vao->enableArrayAttrib(0);
		vao->arrayAttribFormatF(0, 3, 0, 0);
		vao->arrayAttribBinding(0, 0);
		vao->bindVertexBuffer(*vbo, 0, 0, sizeof(vx::float3));

		Graphics::CommandListFactory::createFromFile("commandListEditor.txt", m_objectManager, m_shaderManager, &m_commandList);

		createVoxelSegment();
	}

	void RenderAspect::shutdown(void* hwnd)
	{
		m_meshManager.shutdown();
		m_materialManager.shutdown();
		m_objectManager.shutdown();
		m_coldData.reset(nullptr);
		m_renderContext.shutdown((HWND)hwnd);
	}

	bool RenderAspect::initializeProfiler(Logfile* errorlog)
	{
		return true;
	}

	void RenderAspect::makeCurrent(bool b)
	{

	}

	void RenderAspect::queueUpdate(RenderUpdateTaskType type, const u8* data, u32 dataSize)
	{

	}

	void RenderAspect::queueUpdateCamera(const RenderUpdateCameraData &data)
	{

	}

	void RenderAspect::updateCamera()
	{
		auto projectionMatrix = m_projectionMatrix;

		vx::mat4d viewMatrixD;
		m_camera.getViewMatrixRH(&viewMatrixD);

		vx::mat4 viewMatrix;
		viewMatrixD.asFloat(&viewMatrix);

		UniformCameraBufferBlock block;
		block.position = _mm256_cvtpd_ps(m_camera.getPosition());
		block.viewMatrix = viewMatrix;
		block.pvMatrix = projectionMatrix * block.viewMatrix;
		block.inversePVMatrix = vx::MatrixInverse(block.pvMatrix);

		m_cameraBuffer.subData(0, sizeof(UniformCameraBufferBlock), &block);
	}

	void RenderAspect::update()
	{
		updateCamera();
	}

	void RenderAspect::updateProfiler(f32 dt)
	{

	}

	void RenderAspect::buildCommands()
	{

	}

	void RenderAspect::submitCommands()
	{
		vx::gl::StateManager::bindFrameBuffer(0);
		vx::gl::StateManager::setClearColor(0.1f, 0.1f, 0.1f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (m_selectedInstance.ptr != nullptr)
		{
			auto cmdBuffer = m_objectManager.getBuffer("meshCmdBuffer");
			auto meshVao = m_objectManager.getVertexArray("meshVao");

			vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);

			vx::gl::StateManager::bindVertexArray(*meshVao);

			vx::gl::StateManager::bindBuffer(vx::gl::BufferType::Draw_Indirect_Buffer, cmdBuffer->getId());

			auto offset = m_selectedInstance.cmd.baseInstance * sizeof(vx::gl::DrawElementsIndirectCommand);

			auto pipe = m_shaderManager.getPipeline("editorSelectedMesh.pipe");
			vx::gl::StateManager::bindPipeline(*pipe);

			glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void*)offset);
		}

		m_commandList.draw();

		/*{
			auto emptyVao = m_objectManager.getVertexArray("emptyVao");
			auto pipeline = m_shaderManager.getPipeline("editorDrawVoxelGrid.pipe");

			vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);
			vx::gl::StateManager::enable(vx::gl::Capabilities::Blend);
			vx::gl::StateManager::bindVertexArray(*emptyVao);
			vx::gl::StateManager::bindPipeline(*pipeline);

			glLineWidth(1.0f);

			glDrawArraysInstanced(GL_POINTS, 0, 64, 64 * 64);
		}*/
	}

	void RenderAspect::swapBuffers()
	{
		m_renderContext.swapBuffers();
	}

	void RenderAspect::wait()
	{
		glFinish();
	}

	void RenderAspect::handleMessage(const vx::Message &evt)
	{
		switch (evt.type)
		{
		case(vx::MessageType::File) :
			handleFileMessage(evt);
			break;
		case(vx::MessageType::Editor) :
			//handleEditorEvent(evt);
			break;
		case(vx::MessageType::Renderer) :
			break;
		default:
			break;
		}
	}

	void RenderAspect::keyPressed(u16 key)
	{

	}

	void RenderAspect::getProjectionMatrix(vx::mat4* m) const
	{
		*m = m_projectionMatrix;
	}

	void RenderAspect::getTotalVRam(u32* totalVram) const
	{

	}

	void RenderAspect::getTotalAvailableVRam(u32* totalAvailableVram) const
	{

	}

	void RenderAspect::getAvailableVRam(u32* availableVram) const
	{
		*availableVram = 0;
	}

	void RenderAspect::addMeshInstance(const Editor::MeshInstance &instance)
	{
		//m_sceneRenderer.editorAddMeshInstance(instance.getMeshInstance(), m_fileAspect);
		auto material = instance.getMaterial();
		auto materialSid = material->getSid();

		u32 materialIndex = 0;
		auto b = m_materialManager.getMaterialIndex(materialSid, m_resourceAspect, &materialIndex);
		m_meshManager.addMeshInstance(instance.getMeshInstance(), materialIndex, m_resourceAspect);
	}

	bool RenderAspect::removeMeshInstance(const vx::StringID &sid)
	{
		//return m_sceneRenderer.editorRemoveStaticMeshInstance(sid);
		return false;
	}

	bool RenderAspect::setSelectedMeshInstance(const Editor::MeshInstance* instance)
	{
		if (instance)
		{
			vx::gl::DrawElementsIndirectCommand cmd{};
			auto found = m_meshManager.getDrawCommand(instance->getNameSid(), &cmd);

			if (!found)
			{
				return false;
			}

			m_selectedInstance.cmd = cmd;
			m_selectedInstance.ptr = instance;
		}
		else
		{
			m_selectedInstance.ptr = nullptr;
			return false;
		}

		return true;
	}

	void RenderAspect::setSelectedMeshInstanceTransform(vx::Transform &transform)
	{
		if (m_selectedInstance.ptr)
		{
			auto index = m_selectedInstance.cmd.baseInstance;

			m_meshManager.updateTransform(transform, index);
			//m_sceneRenderer.updateTransform(transform, index);
		}
	}

	bool RenderAspect::setSelectedMeshInstanceMaterial(const Material* material)
	{
		u32 materialIndex = 0;
		auto b = m_materialManager.getMaterialIndex(material->getSid(), m_resourceAspect, &materialIndex);
		if (b)
		{
			m_meshManager.setMaterial(m_selectedInstance.ptr->getNameSid(), materialIndex);
		}

		return b;
	}

	bool RenderAspect::setMeshInstanceMesh(const vx::StringID &sid, const vx::StringID &meshSid)
	{
		m_meshManager.setMesh(sid, meshSid, m_resourceAspect);

		return true;
	}

	void RenderAspect::moveCamera(f32 dirX, f32 dirY, f32 dirZ)
	{
		const f64 speed = 0.05;

		__m256d direction = { dirX, dirY, dirZ, 0 };
		m_camera.move(direction, speed);
	}

	void VX_CALLCONV RenderAspect::rotateCamera(const __m128 rotation)
	{
		__m256d ro = _mm256_cvtps_pd(rotation);
		m_camera.setRotation(ro);
	}

	void RenderAspect::showInfluenceMap(bool b, const InfluenceMap &influenceMap)
	{
		auto cmdBuffer = m_objectManager.getBuffer("drawInfluenceCellNewCmd");
		auto mappedCmd = cmdBuffer->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
		if (b)
		{
			mappedCmd->count = m_coldData->m_influenceMapIndexCount;
		}
		else
		{

			mappedCmd->count = 0;
		}
	}

	void RenderAspect::showNavMesh(bool b, const NavMesh &navMesh, const NavMeshGraph &navMeshGraph)
	{
		if (b)
		{
			updateNavMeshIndexBuffer(navMesh);

			auto editorDrawNavmeshConnectionCmd = m_objectManager.getBuffer("editorDrawNavmeshConnectionCmd");
			auto mappedEditorDrawNavmeshConnectionCmd = editorDrawNavmeshConnectionCmd->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
			mappedEditorDrawNavmeshConnectionCmd->count = m_coldData->m_navmeshConnectionIndexCount;
			mappedEditorDrawNavmeshConnectionCmd.unmap();

			auto navmeshVertexCount = navMesh.getVertexCount();
			auto navMeshVertexCmdBuffer = m_objectManager.getBuffer("navMeshVertexCmdBuffer");
			auto mappedNavMeshVertexCmdBuffer = navMeshVertexCmdBuffer->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
			mappedNavMeshVertexCmdBuffer->count = navmeshVertexCount;
			mappedNavMeshVertexCmdBuffer.unmap();

			auto nodeCount = navMeshGraph.getNodeCount();
			auto graphNodesCmdBuffer = m_objectManager.getBuffer("graphNodesCmdBuffer");
			auto mappedGraphNodesCmdBuffer = graphNodesCmdBuffer->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
			mappedGraphNodesCmdBuffer->count = nodeCount;
			mappedGraphNodesCmdBuffer.unmap();
		}
		else
		{
			auto navmeshCmdBuffer = m_objectManager.getBuffer("navmeshCmdBuffer");
			auto mappedNavmeshCmdBuffer = navmeshCmdBuffer->map<vx::gl::DrawElementsIndirectCommand>(vx::gl::Map::Write_Only);
			mappedNavmeshCmdBuffer->count = 0;
			mappedNavmeshCmdBuffer.unmap();

			auto editorDrawNavmeshConnectionCmd = m_objectManager.getBuffer("editorDrawNavmeshConnectionCmd");
			auto mappedCmd = editorDrawNavmeshConnectionCmd->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
			mappedCmd->count = 0;
			mappedCmd.unmap();

			auto navMeshVertexCmdBuffer = m_objectManager.getBuffer("navMeshVertexCmdBuffer");
			auto mappedNavMeshVertexCmdBuffer = navMeshVertexCmdBuffer->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
			mappedNavMeshVertexCmdBuffer->count = 0;
			mappedNavMeshVertexCmdBuffer.unmap();

			auto graphNodesCmdBuffer = m_objectManager.getBuffer("graphNodesCmdBuffer");
			auto mappedGraphNodesCmdBuffer = graphNodesCmdBuffer->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
			mappedGraphNodesCmdBuffer->count = 0;
			mappedGraphNodesCmdBuffer.unmap();
		}
	}

	void RenderAspect::uploadToNavMeshVertexBuffer(const VertexPositionColor* vertices, u32 count)
	{
		auto navMeshVertexVbo = m_objectManager.getBuffer("navMeshVertexVbo");

		auto dst = navMeshVertexVbo->map<VertexPositionColor>(vx::gl::Map::Write_Only);
		::memcpy(dst.get(), vertices, sizeof(VertexPositionColor) * count);
	}

	void RenderAspect::updateNavMeshVertexBufferWithSelectedVertex(const vx::float3* vertices, u32 count, u32(&selectedVertexIndex)[3], u8 selectedCount)
	{
		auto color = vx::float3(1, 0, 0);
		auto src = vx::make_unique<VertexPositionColor[]>(count);
		for (u32 i = 0; i < count; ++i)
		{
			src[i].position = vertices[i];
			src[i].color = color;
		}

		for (u8 i = 0; i < selectedCount; ++i)
		{
			auto index = selectedVertexIndex[i];
			src[index].color.y = 1.0f;
		}

		//m_coldData->m_navMeshVertexCount = count;
		uploadToNavMeshVertexBuffer(src.get(), count);

		auto cmdBuffer = m_objectManager.getBuffer("navMeshVertexCmdBuffer");
		auto mappedCmdBuffer = cmdBuffer->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
		mappedCmdBuffer->count = count;
	}

	void RenderAspect::updateNavMeshBuffer(const NavMesh &navMesh, u32(&selectedVertexIndex)[3], u8 selectedCount)
	{
		auto navMeshVertexCount = navMesh.getVertexCount();
		if (navMeshVertexCount != 0)
		{
			auto vertices = navMesh.getVertices();
			updateNavMeshVertexBufferWithSelectedVertex(vertices, navMeshVertexCount, selectedVertexIndex, selectedCount);
		}

		updateNavMeshIndexBuffer(navMesh);
	}

	void RenderAspect::updateNavMeshBuffer(const NavMesh &navMesh)
	{
		auto navMeshVertexCount = navMesh.getVertexCount();
		if (navMeshVertexCount != 0)
		{
			auto vertices = navMesh.getVertices();
			u32 tmp[3];
			updateNavMeshVertexBufferWithSelectedVertex(vertices, navMeshVertexCount, tmp, 0);
		}

		updateNavMeshIndexBuffer(navMesh);
	}

	void RenderAspect::updateInfluenceCellBuffer(const InfluenceMap &influenceMap)
	{
		auto influenceCells = influenceMap.getCells();
		auto cellCount = influenceMap.getCellCount();
		if (cellCount == 0)
			return;

		auto triangles = influenceMap.getTriangles();
		printf("InfluenceCellsNew: %u\n", cellCount);

		auto vbo = m_objectManager.getBuffer("drawInfluenceCellNewVbo");
		auto mappedBuffer = vbo->map<VertexPositionColor>(vx::gl::Map::Write_Only);

		const vx::float3 colors[] =
		{
			{ 1, 0, 0 },
			{ 0.5f, 1, 0 },
			{ 0.5f, 0, 1 },
			{ 0, 1, 0 },
			{ 1, 0.5f, 0 },
			{ 0, 0.5f, 1 },
			{ 0, 0, 1 },
			{ 0, 1, 0.5f },
			{ 1, 0, 0.5f },
			{ 1, 0.5f, 1 },
			{ 1, 1, 0 },
			{ 0, 1, 1 },
			{ 1, 0, 1 }
		};

		const u32 colorCount = sizeof(colors) / sizeof(vx::float3);

		u32 vertexOffset = 0;
		u32 colorIndex = 0;
		for (u32 i = 0; i < cellCount; ++i)
		{
			auto &cell = influenceCells[i];

			for (u32 k = 0; k < cell.triangleCount; ++k)
			{
				auto triangleIndex = cell.triangleOffset + k;
				auto &triangle = triangles[triangleIndex];

				mappedBuffer[vertexOffset + 0].position = triangle[0];
				mappedBuffer[vertexOffset + 0].color = colors[colorIndex];

				mappedBuffer[vertexOffset + 1].position = triangle[1];
				mappedBuffer[vertexOffset + 1].color = colors[colorIndex];

				mappedBuffer[vertexOffset + 2].position = triangle[2];
				mappedBuffer[vertexOffset + 2].color = colors[colorIndex];

				vertexOffset += 3;
			}

			colorIndex = (colorIndex + 1) % colorCount;
		}

		auto cmdBuffer = m_objectManager.getBuffer("drawInfluenceCellNewCmd");
		auto mappedCmd = cmdBuffer->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
		mappedCmd->count = vertexOffset;

		m_coldData->m_influenceMapIndexCount = vertexOffset;
	}

	void RenderAspect::updateNavMeshGraphNodesBuffer(const NavMeshGraph &navMeshGraph)
	{
		auto nodes = navMeshGraph.getNodes();
		auto nodeCount = navMeshGraph.getNodeCount();

		if (nodeCount == 0)
			return;

		auto src = vx::make_unique < vx::float3[]>(nodeCount);
		for (u32 i = 0; i < nodeCount; ++i)
		{
			src[i] = nodes[i].m_position;
		}

		//	m_navMeshGraphNodesCount = nodeCount;

		auto navMeshGraphNodesVbo = m_objectManager.getBuffer("navMeshGraphNodesVbo");
		auto mappedBuffer = navMeshGraphNodesVbo->map<vx::float3>(vx::gl::Map::Write_Only);
		::memcpy(mappedBuffer.get(), src.get(), sizeof(vx::float3) * nodeCount);

		auto cmdBuffer = m_objectManager.getBuffer("graphNodesCmdBuffer");
		auto mappedCmdBuffer = cmdBuffer->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
		mappedCmdBuffer->count = nodeCount;

		{
			auto connections = navMeshGraph.getConnections();
			auto connectionCount = navMeshGraph.getConnectionCount();
			VX_ASSERT(connectionCount < 1024);

			u32 index = 0;
			auto vbo = m_objectManager.getBuffer("editorDrawNavmeshConnectionVbo");


			auto mappedVbo = vbo->map<vx::float3>(vx::gl::Map::Write_Only);

			for (u32 i = 0; i < nodeCount; ++i)
			{
				auto &currentNode = nodes[i];
				for (u32 j = 0; j < currentNode.m_connectionCount; ++j)
				{
					auto &connection = connections[currentNode.m_connectionOffset + j];
					//VX_ASSERT(otherIndex < nodeCount);
					auto &otherNode = nodes[connection.m_toNode];

					mappedVbo[index++] = currentNode.m_position;
					mappedVbo[index++] = otherNode.m_position;
				}
			}

			auto cmd = m_objectManager.getBuffer("editorDrawNavmeshConnectionCmd");
			auto mappedCmd = cmd->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
			mappedCmd->count = index;

			m_coldData->m_navmeshConnectionIndexCount = index;
		}
	}

	void RenderAspect::updateLightBuffer(const Graphics::Light* lights, u32 count)
	{
		if (count == 0)
			return;

		count = std::min(count, g_maxLightCount);

		auto editorLightBuffer = m_objectManager.getBuffer("editorLightBuffer");
		auto mappedLightBuffer = editorLightBuffer->mapRange<LightData>(sizeof(vx::uint4), sizeof(LightData) * count, vx::gl::MapRange::Write);
		for (u32 i = 0;i < count; ++i)
		{
			mappedLightBuffer[i].position.x = lights[i].m_position.x;
			mappedLightBuffer[i].position.y = lights[i].m_position.y;
			mappedLightBuffer[i].position.z = lights[i].m_position.z;
			mappedLightBuffer[i].falloff = lights[i].m_falloff;
			mappedLightBuffer[i].lumen = lights[i].m_lumen;
		}
		mappedLightBuffer.unmap();

		auto mappedLightBufferCount = editorLightBuffer->mapRange<vx::uint4>(0, sizeof(vx::uint4), vx::gl::MapRange::Write);
		mappedLightBufferCount->x = count;
		mappedLightBufferCount.unmap();

		auto cmdBuffer = m_objectManager.getBuffer("editorLightCmdBuffer");
		auto mappedCmdBuffer = cmdBuffer->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
		mappedCmdBuffer->count = count;
	}

	void RenderAspect::updateWaypoints(const Waypoint* w, u32 count)
	{
		auto vbo = m_objectManager.getBuffer("waypointVbo");
		auto cmd = m_objectManager.getBuffer("waypointCmdBuffer");

		auto mappedVbo = vbo->map<VertexPositionColor>(vx::gl::Map::Write_Only);
		for (u32 i = 0; i < count; ++i)
		{
			mappedVbo[i].position = w[i].position;
			mappedVbo[i].color = { 1, 1, 0 };
		}
		mappedVbo.unmap();

		cmd->subData(0, sizeof(u32), &count);
	}

	void RenderAspect::updateSpawns(const Spawn* spawns, u32 count)
	{
		auto cmdBuffer = m_objectManager.getBuffer("spawnPointCmdBuffer");
		auto mappedCmdBuffer = cmdBuffer->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
		mappedCmdBuffer->count = count;
		mappedCmdBuffer.unmap();

		auto spawnPointVbo = m_objectManager.getBuffer("spawnPointVbo");
		auto mappedVbo = spawnPointVbo->map<vx::float3>(vx::gl::Map::Write_Only);
		for (u32 i = 0; i < count; ++i)
		{
			mappedVbo[i] = spawns[i].position;
		}
	}

	void RenderAspect::updateJoints(const Joint* joints, u32 count, const vx::sorted_vector<vx::StringID, MeshInstance> &meshinstances)
	{
		auto cmdBuffer = m_objectManager.getBuffer("editorJointCmdBuffer");
		auto mappedCmdBuffer = cmdBuffer->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
		mappedCmdBuffer->count = count;
		mappedCmdBuffer.unmap();

		auto editorJointBuffer = m_objectManager.getBuffer("editorJointBuffer");
		auto mappedBuffer = editorJointBuffer->map<JointData>(vx::gl::Map::Write_Only);
		for (u32 i = 0; i < count; ++i)
		{
			auto p0 = vx::loadFloat3(joints[i].p0);

			auto p1 = vx::loadFloat3(joints[i].p1);
			//auto q1 = vx::loadFloat4(joints[i].q1);
			//p1 = vx::quaternionRotation(p1, q1);

			if (joints[i].sid0.value != 0)
			{
				auto it = meshinstances.find(joints[i].sid0);
				auto &transform = it->getTransform();

				auto rotation = vx::loadFloat4(transform.m_qRotation);
				auto translation = vx::loadFloat3(transform.m_translation);

				p0 = vx::quaternionRotation(p0, rotation);
				p0 = _mm_add_ps(p0, translation);
			}

			if (joints[i].sid1.value != 0)
			{
				auto it = meshinstances.find(joints[i].sid1);
				auto &transform = it->getTransform();

				auto rotation = vx::loadFloat4(transform.m_qRotation);
				auto translation = vx::loadFloat3(transform.m_translation);

				p1 = vx::quaternionRotation(p1, rotation);
				p1 = _mm_add_ps(p1, translation);
			}

			vx::storeFloat4(&mappedBuffer[i].p0, p0);
			vx::storeFloat4(&mappedBuffer[i].p1, p1);

		}
	}

	void RenderAspect::updateLightGeometryProxies(const Graphics::LightGeometryProxy* proxies, u32 count)
	{
		if (count != 0)
		{
			auto editorLightGeometryProxyBuffer = m_objectManager.getBuffer("editorLightGeometryProxyBuffer");

			auto gpuPtr = editorLightGeometryProxyBuffer->map<LightGeometryProxyData>(vx::gl::Map::Write_Only);
			for (u32 i = 0; i < count; ++i)
			{
				auto &proxy = proxies[i];
				gpuPtr[i].vmin = vx::float4(proxy.m_bounds.min, 0);
				gpuPtr[i].vmax = vx::float4(proxy.m_bounds.max, 0);
			}
			gpuPtr.unmap();

			vx::gl::DrawArraysIndirectCommand cmd;
			memset(&cmd, 0, sizeof(cmd));

			cmd.instanceCount = 1;
			cmd.count = count;

			auto editorLightGeometryProxyCmdBuffer = m_objectManager.getBuffer("editorLightGeometryProxyCmdBuffer");
			editorLightGeometryProxyCmdBuffer->subData(0, sizeof(cmd), &cmd);
		}
	}

	void RenderAspect::getViewMatrix(vx::mat4* viewMatrix) const
	{
		vx::mat4d tmp;
		m_camera.getViewMatrixRH(&tmp);

		tmp.asFloat(viewMatrix);
	}

	void RenderAspect::getCameraPosition(vx::float4a* position) const
	{
		auto p = m_camera.getPosition();
		*position = _mm256_cvtpd_ps(p);
	}

	void RenderAspect::updateNavMeshIndexBuffer(const u16* indices, u32 triangleCount)
	{
		auto ibo = m_objectManager.getBuffer("navMeshVertexIbo");

		auto dst = ibo->map<u16>(vx::gl::Map::Write_Only);
		::memcpy(dst.get(), indices, sizeof(u16) * triangleCount * 3);
	}

	void RenderAspect::updateNavMeshIndexBuffer(const NavMesh &navMesh)
	{
		auto triangleCount = navMesh.getTriangleCount();
		if (triangleCount != 0)
		{
			auto indices = navMesh.getTriangleIndices();
			updateNavMeshIndexBuffer(indices, triangleCount);

			auto navMeshIndexCount = triangleCount * 3;

			auto navmeshCmdBuffer = m_objectManager.getBuffer("navmeshCmdBuffer");
			auto mappedCmdBuffer = navmeshCmdBuffer->map<vx::gl::DrawElementsIndirectCommand>(vx::gl::Map::Write_Only);
			mappedCmdBuffer->count = navMeshIndexCount;
		}
	}

	void RenderAspect::handleLoadScene(const vx::Message &evt)
	{
		auto scene = (Editor::Scene*)evt.arg2.ptr;

		//m_sceneRenderer.loadScene(scene, m_objectManager, m_fileAspect);
		auto instances = scene->getMeshInstancesEditor();
		auto instanceCount = scene->getMeshInstanceCount();
		for (u32 i = 0; i < instanceCount; ++i)
		{
			auto &instance = instances[i];
			u32 materialIndex = 0;
			auto b = m_materialManager.getMaterialIndex(*instance.getMaterial(), m_resourceAspect, &materialIndex);
			VX_ASSERT(b);

			auto gpuIndex = m_meshManager.addMeshInstance(instance.getMeshInstance(), materialIndex, m_resourceAspect);
		}

		auto lightCount = scene->getLightCount();
		auto lights = scene->getLights();
		updateLightBuffer(lights, lightCount);

		auto &navMesh = scene->getNavMesh();
		updateNavMeshBuffer(navMesh);

		{
			auto cmdBuffer = m_objectManager.getBuffer("editorLightCmdBuffer");
			auto mappedCmdBuffer = cmdBuffer->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
			mappedCmdBuffer->count = lightCount;
			mappedCmdBuffer.unmap();
		}

		auto spawnCount = scene->getSpawnCount();
		auto spawns = scene->getSpawns();
		updateSpawns(spawns, spawnCount);
	}

	void RenderAspect::handleFileMessage(const vx::Message &msg)
	{
		if ((vx::FileMessage)msg.code == vx::FileMessage::EditorScene_Loaded)
		{
			handleLoadScene(msg);
		}
		else if ((vx::FileMessage)msg.code == vx::FileMessage::Mesh_Loaded)
		{
			//handleLoadMesh(evt);
		}
	}

	void RenderAspect::handleRendererMessage(const vx::Message &msg)
	{
	}

	void RenderAspect::testLightGeometryProxies(const TestLightGeometryProxiesDesc &desc)
	{
		auto getShadowTransform = [](const Gpu::LightData &light, ShadowTransform* shadowTransform)
		{
			auto n = 0.1f;
			auto f = light.falloff;

			auto lightPos = light.position;
			auto projectionMatrix = vx::MatrixPerspectiveFovRHDX(vx::degToRad(90.0f), 1.0f, n, f);

			const __m128 upDirs[6] =
			{
				{ 0, -1, 0, 0 },
				{ 0, -1, 0, 0 },
				{ 0, 0, 1, 0 },
				{ 0, 0, -1, 0 },
				{ 0, -1, 0, 0 },
				{ 0, -1, 0, 0 }
			};

			const __m128 dirs[6] =
			{
				{ 1, 0, 0, 0 },
				{ -1, 0, 0, 0 },
				{ 0, 1, 0, 0 },
				{ 0, -1, 0, 0 },
				{ 0, 0, 1, 0 },
				{ 0, 0, -1, 0 }
			};

			__m128 vf = { f, f, f, 0 };
			__m128 pmax = { f, f, f, 1 };
			__m128 pmin = { -f, -f, -f, 1 };
			//auto pmax = _mm_add_ps(lightPos, vf);
			//auto pmin = _mm_sub_ps(lightPos, vf);

			auto p0 = vx::Vector4Transform(projectionMatrix, pmax);
			auto p1 = vx::Vector4Transform(projectionMatrix, pmin);

			auto dx = p0.m128_f32[0] - p1.m128_f32[0];
			auto dy = p0.m128_f32[1] - p1.m128_f32[1];

			auto sx = 2.0f / (dx);
			auto sy = 2.0f / dy;

			auto ox = -(sx * (p0.m128_f32[0] + p1.m128_f32[0])) / 2.0f;
			auto oy = -(sy * (p0.m128_f32[1] + p1.m128_f32[1])) / 2.0f;

			shadowTransform->scaleMatrix.c[0] = { sx, 0, 0, 0 };
			shadowTransform->scaleMatrix.c[1] = { 0, sy, 0, 0 };
			shadowTransform->scaleMatrix.c[2] = { 0, 0, 1, 0 };
			shadowTransform->scaleMatrix.c[3] = { ox, oy, 0, 1 };
			shadowTransform->projectionMatrix = projectionMatrix;
			for (u32 i = 0; i < 6; ++i)
			{
				auto viewMatrix = vx::MatrixLookToRH(lightPos, dirs[i], upDirs[i]);
				shadowTransform->viewMatrix[i] = viewMatrix;
				shadowTransform->pvMatrix[i] = projectionMatrix * viewMatrix;
			}
		};

		struct ResultData
		{
			std::vector<u32> lightIndices;
		};

		auto proxyCount = desc.proxyCount;
		if (proxyCount != 0)
		{
			std::vector<ResultData> resultData;
			resultData.reserve(proxyCount);
			for (u32 i = 0; i < proxyCount; ++i)
			{
				resultData.push_back(ResultData());
			}

			auto proxies = desc.proxies;
			std::vector<AABB> geometry;
			std::vector<s32> masks;
			std::vector<std::pair<u32, u32>> pairs;

			EditorRenderAspectCpp::generateProxyGeometry(desc.proxies, desc.proxyCount, &geometry, &masks, &pairs);

			auto lightCount = desc.lightCount;
			auto lights = desc.lights;

			std::vector<ShadowTransform> shadowTransforms;
			shadowTransforms.reserve(lightCount);
			for (u32 i = 0; i < lightCount; ++i)
			{
				auto &light = lights[i];

				Gpu::LightData data;
				data.position = vx::loadFloat3(light.m_position);
				data.falloff = light.m_falloff;
				data.lumen = light.m_lumen;

				ShadowTransform transform;
				transform.position = data.position;
				transform.falloff_lumen.x = data.falloff;
				transform.falloff_lumen.y = data.lumen;

				getShadowTransform(data, &transform);

				shadowTransforms.push_back(transform);
			}

			auto geometryCount = geometry.size();

			std::vector<u32> lightResultData;
			lightResultData.reserve(geometryCount);
			for (u32 i = 0; i < geometryCount; ++i)
			{
				lightResultData.push_back(0);
			}

			// UNIFORM(ShadowTransformBufferBlock, 5)
			vx::gl::Buffer shadowTransformBuffer;
			{
				vx::gl::BufferDescription bufferDesc;
				bufferDesc.bufferType = vx::gl::BufferType::Shader_Storage_Buffer;
				bufferDesc.immutable = 1;
				bufferDesc.size = sizeof(ShadowTransform) * lightCount;
				bufferDesc.flags = 0;
				bufferDesc.pData = shadowTransforms.data();
				shadowTransformBuffer.create(bufferDesc);
			}
			// layout(binding = 5, shared) buffer 
			vx::gl::Buffer geometryBuffer;
			{
				vx::gl::BufferDescription bufferDesc;
				bufferDesc.bufferType = vx::gl::BufferType::Shader_Storage_Buffer;
				bufferDesc.immutable = 1;
				bufferDesc.size = sizeof(LightGeometryProxyData) * geometryCount;
				bufferDesc.flags = vx::gl::BufferStorageFlags::Write;
				bufferDesc.pData = nullptr;
				geometryBuffer.create(bufferDesc);
			}
			// layout(binding = 6, shared) buffer ResultBufferBlock
			vx::gl::Buffer resultBuffer;
			{
				vx::gl::BufferDescription bufferDesc;
				bufferDesc.bufferType = vx::gl::BufferType::Shader_Storage_Buffer;
				bufferDesc.immutable = 1;
				bufferDesc.size = sizeof(u32) * geometryCount;
				bufferDesc.flags = vx::gl::BufferStorageFlags::Read | vx::gl::BufferStorageFlags::Write | vx::gl::BufferStorageFlags::Dynamic_Storage;
				bufferDesc.pData = nullptr;
				resultBuffer.create(bufferDesc);
			}

			auto gpuPtr = geometryBuffer.map<LightGeometryProxyData>(vx::gl::Map::Write_Only);
			for (u32 i = 0; i < geometryCount; ++i)
			{
				s32 mask = masks[i];

				f32 ftmp = 0.0f;
				s32* ptr = (s32*)&ftmp;
				*ptr = mask;

				gpuPtr[i].vmin = vx::float4(geometry[i].min, ftmp);
				gpuPtr[i].vmax = vx::float4(geometry[i].max, 1);
			}
			gpuPtr.unmap();

			vx::gl::Texture depthTexture;
			vx::gl::Framebuffer fb;
			{
				vx::gl::TextureDescription texDesc;
				texDesc.format = vx::gl::TextureFormat::DEPTH32F;
				texDesc.miplevels = 1;
				texDesc.size = vx::ushort3(1024, 1024, 1);
				texDesc.sparse = 0;
				texDesc.type = vx::gl::TextureType::Texture_Cubemap;
				depthTexture.create(texDesc);

				fb.create();
				fb.attachTexture(vx::gl::Attachment::Depth, depthTexture, 0);
			}

			glBindBufferBase(GL_UNIFORM_BUFFER, 5, shadowTransformBuffer.getId());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, geometryBuffer.getId());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, resultBuffer.getId());

			auto pipeline = m_shaderManager.getPipeline("editorTestLightGeometryProxy.pipe");
			auto pipeline0 = m_shaderManager.getPipeline("editorTestLightGeometryProxy0.pipe");
			auto emptyVao = m_objectManager.getVertexArray("emptyVao");

			fb.bind();
			emptyVao->bind();
			
			vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Clamp);
			vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);
			vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);

			vx::gl::StateManager::setViewport(0, 0, 1024, 1024);

			glClearDepth(1.0f);
			for (u32 lightIndex = 0; lightIndex < lightCount; ++lightIndex)
			{
				auto ptrResults = resultBuffer.map<u32>(vx::gl::Map::Write_Only);
				for (u32 j = 0; j < geometryCount; ++j)
				{
					ptrResults[j] = 0;
				}
				ptrResults.unmap();

				glClear(GL_DEPTH_BUFFER_BIT);

				pipeline0->bind();
				glProgramUniform1ui(pipeline0->getGeometryShader(), 0, lightIndex);
				glCullFace(GL_FRONT);
				glDrawArrays(GL_POINTS, 0, geometryCount);

				pipeline->bind();
				glProgramUniform1ui(pipeline->getGeometryShader(), 0, lightIndex);

				glCullFace(GL_BACK);
				glDrawArrays(GL_POINTS, 0, geometryCount);

				ptrResults = resultBuffer.map<u32>(vx::gl::Map::Read_Only);
				for (u32 j = 0; j < geometryCount; ++j)
				{
					lightResultData[j] = ptrResults[j];
				}
				ptrResults.unmap();

				auto &light = lights[lightIndex];
				//printf("\nLight %u, %f %f %f\n", lightIndex, light.m_position.x, light.m_position.y, light.m_position.z);

				std::vector<u32> proxyResultData;
				proxyResultData.reserve(proxyCount);
				for (u32 j = 0; j < proxyCount; ++j)
				{
					proxyResultData.push_back(0);
				}

				for (u32 k = 0; k < geometryCount; ++k)
				{
					auto &aabb = geometry[k];
					auto mask = masks[k];
					auto result = lightResultData[k];
					auto proxyPair = pairs[k];

					if (result != 0)
					{
						auto &proxy0 = proxies[proxyPair.first];
						auto &proxy1 = proxies[proxyPair.second];

						if (proxy0.m_bounds.intersects(light.m_position, light.m_falloff))
						{
							proxyResultData[proxyPair.first] = 1;
						}

						if (proxy1.m_bounds.intersects(light.m_position, light.m_falloff))
						{
							proxyResultData[proxyPair.second] = 1;
						}
					}
				}

				for (u32 j = 0; j < proxyCount; ++j)
				{
					if (proxyResultData[j] != 0)
					{
						resultData[j].lightIndices.push_back(lightIndex);
					}
				}
			}

			for (u32 i = 0; i < proxyCount; ++i)
			{
				auto &data = resultData[i];
				auto proxyLightCount = data.lightIndices.size();
				if (proxyLightCount >= 10)
				{
					VX_ASSERT(false);
				}

				for (u32 idx = 0; idx < proxyLightCount; ++idx)
				{
					proxies[i].m_lightIndices[idx] = data.lightIndices[idx];
				}
				proxies[i].m_lightCount = proxyLightCount;
			}

			vx::gl::StateManager::setViewport(0, 0, m_resolution.x, m_resolution.y);
			glCullFace(GL_BACK);
		}
	}
}