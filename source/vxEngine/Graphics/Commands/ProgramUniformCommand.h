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

#include "../Commands.h"

namespace Graphics
{
	struct ProgramUniformCommand : public Command
	{
		U32 m_program;
		vx::gl::DataType m_dataType;
		U8 m_padding;
		U16 m_count;
		U32 m_location;

		void setUInt(U32 program, U32 location, U32 count);
		void setFloat(U32 program, U32 location, U32 count);

		void execute(U32* offset) override;

	private:
		void programUniformFloat(U32* offset);
		void programUniformUInt(U32* offset);
	};

	template<typename T>
	struct ProgramUniformData
	{
		enum { Count = sizeof(T) };

		U8 u[Count];

		ProgramUniformData() : u()
		{
		}

		void set(const T &data)
		{
			memcpy(u, &data, sizeof(T));
		}

		U8& operator[](U32 i)
		{
			return u[i];
		}

		const U8& operator[](U32 i) const
		{
			return u[i];
		}
	};
}