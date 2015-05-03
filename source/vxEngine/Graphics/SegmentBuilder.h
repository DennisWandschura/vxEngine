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