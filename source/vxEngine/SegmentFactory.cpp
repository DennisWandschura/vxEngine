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
#include "SegmentFactory.h"
#include "Graphics/Segment.h"
#include "gl/ObjectManager.h"
#include <vxLib/gl/ShaderManager.h>
#include <vxLib/gl/ProgramPipeline.h>
#include <vxLib/gl/Buffer.h>
#include "ParserNode.h"
#include "Graphics/CommandFactory.h"

namespace Graphics
{
	Graphics::Segment SegmentFactory::createFromFile(const char* file, const gl::ObjectManager &objectManager, const vx::gl::ShaderManager &shaderManager)
	{
		Graphics::Segment segment;

		Parser::Node root;
		root.createFromFile(file);

		std::string vaoName;
		root.get("vao")->as(&vaoName);

		std::string fboName;
		root.get("fbo")->as(&fboName);

		std::string pipelineName;
		root.get("pipeline")->as(&pipelineName);

		std::string cmdName;
		root.get("cmdBuffer")->as(&cmdName);

		std::string paramName;
		root.get("paramBuffer")->as(&paramName);

		bool depthState = true;
		root.get("depthState")->as(&depthState);

		bool blendState = false;
		root.get("blendState")->as(&blendState);

		bool polygonOffsetFillState = false;
		auto polygonOffsetFillStateNode = root.get("polygonOffsetFillState");
		if (polygonOffsetFillStateNode)
		{
			polygonOffsetFillStateNode->as(&polygonOffsetFillState);
		}

		auto vao = objectManager.getVertexArray(vaoName.c_str());
		auto pipe = shaderManager.getPipeline(pipelineName.c_str());
		auto cmdBuffer = objectManager.getBuffer(cmdName.c_str());
		auto paramBuffer = objectManager.getBuffer(paramName.c_str());
		auto fbo = objectManager.getFramebuffer(fboName.c_str());

		auto comamndsNode = root.get("commands");

		auto commandCount = comamndsNode->size();
		for (u32 i = 0; i < commandCount; ++i)
		{
			Parser::Node entry;
			comamndsNode->as(i, &entry);

			CommandFactory::get().createFromNodeAndPushToSegment(entry, &segment, pipe);
		}

		Graphics::StateDescription desc;
		desc.fbo = (fbo != nullptr) ? fbo->getId() : 0;
		desc.vao = vao->getId();
		desc.pipeline = pipe->getId();
		desc.indirectBuffer = cmdBuffer->getId();
		desc.paramBuffer = (paramBuffer != nullptr) ? paramBuffer->getId() : 0;
		desc.depthState = depthState;
		desc.blendState = blendState;
		desc.polygonOffsetFillState = polygonOffsetFillStateNode;

		Graphics::State state;
		state.set(desc);

		segment.setState(state);

		VX_ASSERT(segment.isValid());

		return segment;
	}
}