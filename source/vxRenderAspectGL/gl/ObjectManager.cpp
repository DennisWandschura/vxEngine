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

#include "ObjectManager.h"
#include <vxGL/Texture.h>
#include <vxGL/Buffer.h>
#include <vxGL/VertexArray.h>
#include <vxGL/Framebuffer.h>

namespace gl
{
	ObjectManager::ObjectManager()
		:m_usedMemoryBuffer(0)
	{

	}

	ObjectManager::~ObjectManager()
	{

	}

	void ObjectManager::initialize(u32 maxBufferCount, u32 maxVaoCount, u32 maxFramebufferCount, u32 maxTextureCount, vx::StackAllocator* allocator)
	{
		m_bufferManager.initialize(maxBufferCount, allocator);
		m_vertexArrays = vx::sorted_array<vx::StringID, vx::gl::VertexArray>(maxVaoCount, allocator);
		m_frameBuffers = vx::sorted_array<vx::StringID, vx::gl::Framebuffer>(maxFramebufferCount, allocator);
		m_textures = vx::sorted_array<vx::StringID, vx::gl::Texture>(maxTextureCount, allocator);
	}

	void ObjectManager::shutdown()
	{
		m_bufferManager.shutdown();
		m_vertexArrays.cleanup();
		m_frameBuffers.cleanup();
		m_textures.cleanup();
	}

	vx::StringID ObjectManager::createVertexArray(const char* id)
	{
		auto sid = vx::make_sid(id);
		auto it = m_vertexArrays.find(sid);
		VX_ASSERT(it == m_vertexArrays.end());

		vx::gl::VertexArray vao;
		vao.create();

		it = m_vertexArrays.insert(std::move(sid), std::move(vao));
		if (it == m_vertexArrays.end())
			sid = 0;

		return sid;
	}

	const vx::gl::VertexArray* ObjectManager::getVertexArray(const vx::StringID &sid) const
	{
		const vx::gl::VertexArray* result = nullptr;

		auto it = m_vertexArrays.find(sid);
		if (it != m_vertexArrays.end())
			result = &*it;

		return result;
	}

	const vx::gl::VertexArray* ObjectManager::getVertexArray(const char* id) const
	{
		auto sid = vx::make_sid(id);
		return getVertexArray(sid);
	}

	vx::gl::VertexArray* ObjectManager::getVertexArray(const vx::StringID &sid)
	{
		vx::gl::VertexArray* result = nullptr;

		auto it = m_vertexArrays.find(sid);
		if (it != m_vertexArrays.end())
			result = &*it;

		return result;
	}

	vx::gl::VertexArray* ObjectManager::getVertexArray(const char* id)
	{
		auto sid = vx::make_sid(id);
		return getVertexArray(sid);
	}

	vx::StringID ObjectManager::createFramebuffer(const char* id)
	{
		auto sid = vx::make_sid(id);
		auto it = m_frameBuffers.find(sid);
		VX_ASSERT(it == m_frameBuffers.end());

		vx::gl::Framebuffer fb;
		fb.create();

		it = m_frameBuffers.insert(std::move(sid), std::move(fb));
		if (it == m_frameBuffers.end())
			sid = 0;

		return sid;
	}

	const vx::gl::Framebuffer* ObjectManager::getFramebuffer(const vx::StringID &sid) const
	{
		const vx::gl::Framebuffer* result = nullptr;

		auto it = m_frameBuffers.find(sid);
		if (it != m_frameBuffers.end())
		{
			result = &*it;
		}

		return result;
	}

	const vx::gl::Framebuffer* ObjectManager::getFramebuffer(const char* id) const
	{
		auto sid = vx::make_sid(id);
		return getFramebuffer(sid);
	}

	vx::StringID ObjectManager::createBuffer(const char* key, const vx::gl::BufferDescription &desc)
	{
		auto sid = m_bufferManager.createBuffer(key, desc);

		if (sid.value != 0)
		{
			m_usedMemoryBuffer += desc.size;
		}

		return sid;
	}

	const vx::gl::Buffer* ObjectManager::getBuffer(const vx::StringID &sid) const
	{
		return m_bufferManager.getBuffer(sid);
	}

	const vx::gl::Buffer* ObjectManager::getBuffer(const char* id) const
	{
		return m_bufferManager.getBuffer(id);
	}

	vx::StringID ObjectManager::createTexture(const char* id, const vx::gl::TextureDescription &desc, bool makeResident)
	{
		auto tmpSid = vx::make_sid(id);
		auto it = m_textures.find(tmpSid);
		VX_ASSERT(it == m_textures.end());

		vx::gl::Texture texture;
		texture.create(desc);

		vx::StringID sid{};

		it = m_textures.insert(std::move(tmpSid), std::move(texture));
		if (it != m_textures.end())
		{
			if (makeResident)
			{
				it->makeTextureResident();
			}
			sid = tmpSid;
		}

		return sid;
	}

	const vx::gl::Texture* ObjectManager::getTexture(const vx::StringID &sid) const
	{
		const vx::gl::Texture* result = nullptr;

		auto it = m_textures.find(sid);
		if (it != m_textures.end())
			result = &*it;

		return result;
	}

	const vx::gl::Texture* ObjectManager::getTexture(const char* id) const
	{
		auto sid = vx::make_sid(id);
		return getTexture(sid);
	}

	u32 ObjectManager::getUsedMemoryBuffer() const
	{
		return m_usedMemoryBuffer;
	}
}