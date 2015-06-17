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

namespace vx
{
	namespace gl
	{
		struct DrawElementsIndirectCommand;
	}
}

#include "Renderer.h"
#include <vxLib/memory.h>
#include <vxLib/StringID.h>

namespace Graphics
{
	class Segment;

	class ShadowRenderer : public Renderer
	{
		std::unique_ptr<u32[]> m_shadowDepthTextureIds;
		vx::StringID m_lightCmdBufferSid;
		u32 m_textureCount;

		void createShadowTextureBuffer();
		void createShadowTextures();
		void createFramebuffer();

		void createLightDrawCommandBuffers();

		Segment createSegmentResetCmdBuffer() const;
		Segment createSegmentCullMeshes() const;

	public:
		ShadowRenderer();
		~ShadowRenderer();

		void initialize() override;

		void update() override;

		void updateDrawCmds();
		void updateDrawCmd(const vx::gl::DrawElementsIndirectCommand &cmd, u32 index);

		void getCommandList(CommandList* cmdList) override;

		void clearData() override;
		void bindBuffers() override;

		const u32* getTextureIds() const;
	};
}