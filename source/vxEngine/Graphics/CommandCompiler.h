#pragma once

#include <vxLib/types.h>
#include <string>

namespace Graphics
{
	enum class CommandHeader : U32;

	class CommandCompiler
	{
	public:
		static void getNextCommand(CommandHeader* ptr, U32* offset);
		static void compileCommand(CommandHeader* ptr, U32* offset, std::string* srcFile, U8* dataBuffer, U32* bufferOffset);
	};
}