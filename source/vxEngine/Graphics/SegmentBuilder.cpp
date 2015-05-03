#include "SegmentBuilder.h"
#include "Commands.h"
#include "CommandCompiler.h"
#include <fstream>
#include <process.h>
#include <Windows.h>

namespace
{
	const char* g_segmentsSourceFile = "drawSegments.cpp";
	const char* g_segmentsHeaderFile = "drawSegments.h";
	const char* g_segmentsDllFile = "drawSegments.dll";
}

namespace Graphics
{
	void* SegmentBuilder::s_dllHandle{nullptr};

	std::string SegmentBuilder::s_allSegmentsSourceData
	{
		"#include \"drawSegments.h\"\n#include <vxLib/gl/StateManager.h>\n#include <vxLib/gl/gl.h>\n"
		"#pragma comment(lib, \"opengl32.lib\")\n"
		"#pragma comment(lib, \"vxLib_d.lib\")\n"
		"#pragma comment(lib, \"user32.lib\")\n"
		"#pragma comment(lib, \"Gdi32.lib\")\n"
		"#pragma comment(lib, \"Shlwapi.lib\")\n"
		/*"#pragma comment(lib, \"kernel32.lib\")\n"
		"#pragma comment(lib, \"winspool.lib\")\n"
		"#pragma comment(lib, \"comdlg32.lib\")\n"
		"#pragma comment(lib, \"advapi32.lib\")\n"
		"#pragma comment(lib, \"shell32.lib\")\n"
		"#pragma comment(lib, \"ole32.lib\")\n"
		"#pragma comment(lib, \"oleaut32.lib\")\n"
		"#pragma comment(lib, \"uuid.lib\")\n"
		"#pragma comment(lib, \"odbc32.lib\")\n"
		"#pragma comment(lib, \"odbccp32.lib\")\n"*/
		"BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved )\n{\n"
		"switch( fdwReason )\n"
		"{\n"
		"case DLL_PROCESS_ATTACH:\n"
		"puts(\"process attach\");\n"
		"break;\n"
		"case DLL_THREAD_ATTACH:\n"
		"puts(\"thread attach\");\n"
		"return FALSE;\n"
		"break;\n"
		"case DLL_THREAD_DETACH:\n"
		"puts(\"thread detach\");\n"
		"break;\n"
		"case DLL_PROCESS_DETACH:\n"
		"puts(\"process detach\");"
		"break;\n"
		"}\n"
		"return TRUE;\n}\n"
	};

	std::string SegmentBuilder::s_allSegmentsHeaderData{ "#pragma once\n" };

	SegmentBuilder::SegmentBuilder()
		:m_commmands(),
		m_state()
	{

	}

	SegmentBuilder::~SegmentBuilder()
	{

	}

	void SegmentBuilder::pushCommand(const U8* ptr, U32 count)
	{
		for (U32 i = 0; i < count; ++i)
		{
			m_commmands.push_back(ptr[i]);
		}
	}

	void SegmentBuilder::setState(const State &state)
	{
		m_state = state;
	}

	U32 SegmentBuilder::getDataTypeSize(vx::gl::DataType dataType) const
	{
		U32 dataTypeSize = 0;
		switch (dataType)
		{
		case vx::gl::DataType::Float:
			dataTypeSize = 4;
			break;
		case vx::gl::DataType::Int:
			dataTypeSize = 4;
			break;
		case vx::gl::DataType::Unsigned_Int:
			dataTypeSize = 4;
			break;
		default:
			VX_ASSERT(false);
			break;
		}

		return dataTypeSize;
	}

	U32 SegmentBuilder::getTotalBufferSize() const
	{
		auto count = m_commmands.size();
		U32 totalBufferSizeInBytes = 0;
		for (U32 i = 0; i < count;)
		{
			CommandHeader header = (CommandHeader)m_commmands[i];

			if (header == CommandHeader::ProgramUniformCommand)
			{
				ProgramUniformCommand* cmd = (ProgramUniformCommand*)&m_commmands[i];

				U32 dataTypeSize = getDataTypeSize(cmd->m_dataType);

				if (cmd->m_count > 1)
				{
					auto sizeInBytes = cmd->m_count * dataTypeSize;
					totalBufferSizeInBytes += sizeInBytes;
				}
			}

			U32 offset = 0;
			CommandCompiler::getNextCommand((CommandHeader*)&m_commmands[i], &offset);
			i += offset;
		}

		return totalBufferSizeInBytes;
	}

	void SegmentBuilder::writeDataToBuffer(U8* buffer)
	{
		U32 bufferOffset = 0;

		auto count = m_commmands.size();
		for (U32 i = 0; i < count;)
		{
			CommandHeader header = (CommandHeader)m_commmands[i];
			if (header == CommandHeader::ProgramUniformCommand)
			{
				ProgramUniformCommand* cmd = (ProgramUniformCommand*)&m_commmands[i];

				U32 dataTypeSize = getDataTypeSize(cmd->m_dataType);

				if (cmd->m_count > 1)
				{
					auto sizeInBytes = cmd->m_count * dataTypeSize;
					auto dataPtr = (U8*)(cmd + 1);
					memcpy(buffer + bufferOffset, dataPtr, sizeInBytes);

					bufferOffset += sizeInBytes;
				}
			}

			U32 offset = 0;
			CommandCompiler::getNextCommand((CommandHeader*)&m_commmands[i], &offset);
			i += offset;
		}
	}

	void SegmentBuilder::appendSegment(const char* name, std::unique_ptr<U8[]>* dataBuffer)
	{
		std::string functionName = name;

		std::string segmentSourceData;

		segmentSourceData += "void " + functionName + "()\n{\n";
		m_state.compile(&segmentSourceData);

		auto totalBufferSizeInBytes = getTotalBufferSize();
		*dataBuffer = std::make_unique<U8[]>(totalBufferSizeInBytes);

		writeDataToBuffer(dataBuffer->get());
		U32 bufferOffset = 0;
		auto count = m_commmands.size();
		for (U32 i = 0; i < count;)
		{
			CommandHeader* header = (CommandHeader*)&m_commmands[i];
			U32 offset = 0;

			CommandCompiler::compileCommand(header, &offset, &segmentSourceData, dataBuffer->get(), &bufferOffset);

			i += offset;
		}

		segmentSourceData += "}\n";

		s_allSegmentsSourceData += segmentSourceData;

		s_allSegmentsHeaderData += "extern \"C\" __declspec( dllexport ) void ";
		s_allSegmentsHeaderData += functionName;
		s_allSegmentsHeaderData += "();\n";
	}

	void SegmentBuilder::compile()
	{
		std::ofstream outFile(g_segmentsSourceFile);
		outFile << s_allSegmentsSourceData;
		outFile.close();

		outFile.open(g_segmentsHeaderFile);
		outFile << s_allSegmentsHeaderData;
		outFile.close();

		/*const char *argv[] =
		{
			"/E:ON /V:ON /K \"\"C:/Program Files (x86)/Intel/Composer XE 2015/bin/vars.bat\" intel64 vs2013 & icl /I\"d:/Users/dw/Documents/Visual Studio 2013/Projects/vxLib/include\" /D \"_VX_WINDOWS\" /D \"_UNICODE\" /D \"UNICODE\" /Qstd=c++11",
			g_segmentsSourceFile,
			"/MD /LD /link /LIBPATH:\"d:/Users/dw/Documents/Visual Studio 2013/Projects/vxLib/lib\"\"",
			0
		};
		_execv("C:/Windows/SysWOW64/cmd.exe", argv);*/

		std::string cmd = "\"\"C:/Program Files (x86)/Intel/Composer XE 2015/bin/vars.bat\" intel64 vs2013 & icl /I\"d:/Users/dw/Documents/Visual Studio 2013/Projects/vxLib/include\" /D \"_VX_WINDOWS\" /D \"_UNICODE\" /D \"UNICODE\" /Qstd=c++11 ";
		cmd += g_segmentsSourceFile;
		cmd += " /MDd /LDd /Od /link /DEBUG /LIBPATH:\"d:/Users/dw/Documents/Visual Studio 2013/Projects/vxLib/lib\"\"";
		system(cmd.c_str());

		s_dllHandle = LoadLibraryA(g_segmentsDllFile);
		//fp = (ProcSig)GetProcAddress(hGetProcIDDLL, m_functionName.c_str());
	}

	void SegmentBuilder::closeDll()
	{
		if (s_dllHandle)
		{
			FreeLibrary((HMODULE)s_dllHandle);
			s_dllHandle = nullptr;
		}
	}

	void* SegmentBuilder::getFunctionPtr(const char* functionName)
	{
		void* fp = nullptr;
		if (s_dllHandle)
		{
			fp = GetProcAddress((HINSTANCE)s_dllHandle, functionName);
		}

		return fp;
	}
}