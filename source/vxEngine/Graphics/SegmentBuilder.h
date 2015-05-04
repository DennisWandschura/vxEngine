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
#pragma once

#include <vector>
#include "State.h"
#include <memory>

namespace vx
{
	namespace gl
	{
		enum class DataType : U8;
	}
}

namespace Graphics
{
	class ProgramUniformCommand;

	template<typename T>
	class ProgramUniformData;

	class SegmentBuilder
	{
		static void* s_dllHandle;
		static std::string s_allSegmentsSourceData;
		static std::string s_allSegmentsHeaderData;

		std::vector<U8> m_commmands;
		State m_state;

		void pushCommand(const U8*, U32 count);

		U32 getDataTypeSize(vx::gl::DataType dataType) const;
		U32 getTotalBufferSize() const;
		void writeDataToBuffer(U8* buffer);

	public:
		SegmentBuilder();
		~SegmentBuilder();

		void setState(const State &state);

		template < typename T >
		std::enable_if<!std::is_same<T, ProgramUniformCommand>::value, void>::type
			pushCommand(const T &command)
		{
			U8* ptr = (U8*)&command;

			pushCommand(ptr, sizeof(T));
		}

		template < typename T >
		void pushCommand(const ProgramUniformCommand &command, const ProgramUniformData<T> &data)
		{
			U8* ptr = (U8*)&command;
			pushCommand(ptr, sizeof(ProgramUniformCommand));
			pushCommand(data.u, sizeof(T));
		}

		void appendSegment(const char* name, std::unique_ptr<U8[]>* dataBuffer);

		static void compile();
		static void closeDll();
		static void* getFunctionPtr(const char* functionName);
	};
}