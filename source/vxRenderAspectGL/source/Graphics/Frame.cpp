#include "vxRenderAspect/Graphics/Frame.h"
#include "vxRenderAspect/Graphics/CommandList.h"

namespace Graphics
{
	Frame::Frame()
	{

	}

	Frame::~Frame()
	{

	}

	void Frame::clear()
	{
		m_commandLists.clear();
	}

	void Frame::pushCommandList(CommandList &&cmdList)
	{
		m_commandLists.push_back(std::move(cmdList));
	}

	void Frame::draw() const
	{
		for (auto &it : m_commandLists)
		{
			it.draw();
		}
	}
}