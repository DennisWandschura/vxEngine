#pragma once

#include <vector>
#include "State.h"
#include <memory>

namespace Graphics
{
	class ProgramUniformCommand;

	template<typename T>
	class ProgramUniformData;

	class Segment
	{
		std::vector<U8> m_commmands;
		State m_state;

		void pushCommand(const U8*, U32 count);

	public:
		Segment();
		~Segment();

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

		void draw();
	};

	class SegmentCompiled
	{
		typedef void(*DrawFunctionType)();

		DrawFunctionType m_drawFunctionFp;
		std::unique_ptr<U8[]> m_buffer;

	public:
		SegmentCompiled();
		SegmentCompiled(SegmentCompiled &&rhs);
		~SegmentCompiled();

		SegmentCompiled& operator=(SegmentCompiled &&rhs);

		void set(void* fp, std::unique_ptr<U8[]> &&buffer);

		void draw();
	};
}