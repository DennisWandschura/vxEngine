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

#include "State.h"
#include <vector>
#include <memory>

namespace Graphics
{
	struct ProgramUniformCommand;

	class Segment
	{
		std::vector<u8> m_commmands;
		State m_state;

		void pushCommandImp(const u8*, u32 count);

	public:
		Segment();
		~Segment();

		void setState(const State &state);

		template < typename T, typename = typename std::enable_if < !std::is_same < T, ProgramUniformCommand>::value>::type >
		void pushCommand(const T &command)
		{
			static_assert(sizeof(T) >= 4u, "");

			CommandFunctionType fn = &T::execute;
			auto address = std::size_t(fn);

			const u8* ptr = (u8*)&address;
			pushCommandImp(ptr, sizeof(std::size_t));

			ptr = (u8*)&command;
			pushCommandImp(ptr, sizeof(T));
		}

		void pushCommand(const ProgramUniformCommand &command, const u8* data);

		void draw() const;

		bool isValid() const;
	};
}