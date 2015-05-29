#include "SegmentFactory.h"
#include "Graphics/Segment.h"
#include "gl/ObjectManager.h"
#include <vxLib/gl/ShaderManager.h>
#include <vxLib/gl/ProgramPipeline.h>
#include <vxLib/gl/Buffer.h>
#include "ParserNode.h"

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

		auto vao = objectManager.getVertexArray(vaoName.c_str());
		auto pipe = shaderManager.getPipeline(pipelineName.c_str());
		auto cmdBuffer = objectManager.getBuffer(cmdName.c_str());
		auto paramBuffer = objectManager.getBuffer(paramName.c_str());
		auto fbo = objectManager.getFramebuffer(fboName.c_str());

		Graphics::StateDescription desc;
		desc.fbo = (fbo != nullptr) ? fbo->getId() : 0;
		desc.vao = vao->getId();
		desc.pipeline = pipe->getId();
		desc.indirectBuffer = cmdBuffer->getId();
		desc.paramBuffer = (paramBuffer != nullptr) ? paramBuffer->getId() : 0;
		desc.depthState = depthState;
		desc.blendState = blendState;

		Graphics::State state;
		state.set(desc);

		segment.setState(state);

		return segment;
	}
}