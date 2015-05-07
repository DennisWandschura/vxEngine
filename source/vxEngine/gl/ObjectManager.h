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

#include "BufferManager.h"
#include <vxLib/gl/VertexArray.h>
#include <vxLib/gl/Framebuffer.h>
#include <vxLib/gl/Texture.h>

namespace gl
{
	class ObjectManager
	{
		BufferManager m_bufferManager;
		vx::sorted_array<vx::StringID, vx::gl::VertexArray> m_vertexArrays;
		vx::sorted_array<vx::StringID, vx::gl::Framebuffer> m_frameBuffers;
		vx::sorted_array<vx::StringID, vx::gl::Texture> m_textures;

	public:
		void initialize(U32 maxBufferCount, U32 maxVaoCount, U32 maxFramebufferCount, U32 maxTextureCount, vx::StackAllocator* allocator);
		void shutdown();

		vx::StringID createVertexArray(const char* id);
		vx::gl::VertexArray* getVertexArray(const vx::StringID &sid) const;
		vx::gl::VertexArray* getVertexArray(const char* id) const;

		vx::StringID createFramebuffer(const char* id);
		vx::gl::Framebuffer* getFramebuffer(const vx::StringID &sid) const;
		vx::gl::Framebuffer* getFramebuffer(const char* id) const;

		vx::StringID createBuffer(const char* id, const vx::gl::BufferDescription &desc);
		vx::gl::Buffer* getBuffer(const vx::StringID &sid) const;
		vx::gl::Buffer* getBuffer(const char* id) const;

		vx::StringID createTexture(const char* id, const vx::gl::TextureDescription &desc);
		vx::gl::Texture* getTexture(const vx::StringID &sid) const;
		vx::gl::Texture* getTexture(const char* id) const;
	};
}