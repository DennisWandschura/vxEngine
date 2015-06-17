#include "Frame.h"
#include "CommandList.h"

namespace Graphics
{
	Frame::Frame()
	{

	}

	Frame::~Frame()
	{

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