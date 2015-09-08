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

#include <vxLib/types.h>

namespace Graphics
{
	enum class CommandApiType : u32 { D3D, GL };

	class CommandList
	{
		CommandApiType m_type;

	public:
		explicit CommandList(CommandApiType type):m_type(type){ }
		CommandList(CommandList&) = delete;
		CommandList(CommandList &&rhs) :m_type(rhs.m_type) {}

		virtual ~CommandList() {}

		CommandList& operator=(const CommandList&) = delete;
		CommandList& operator=(CommandList &&rhs)
		{
			auto tmp = m_type;
			m_type = rhs.m_type;
			rhs.m_type = tmp;

			return *this;
		}

		CommandApiType getApiType() const { return m_type; }
	};
}