#include "File.h"
#include <Windows.h>
#include <strsafe.h>

namespace detail
{
	DWORD g_access[] = { GENERIC_READ, GENERIC_WRITE, GENERIC_READ | GENERIC_WRITE };
}

void printError(DWORD error)
{
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, 0, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, 0);

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + 40)* sizeof(TCHAR));

	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("Failed with error %d: %s"),
		error, lpMsgBuf);

	printf("%ws\n", (wchar_t*)lpDisplayBuf);
}

File::File()
	:m_pFile(nullptr)
{

}

File::~File()
{
	if (m_pFile != nullptr)
		close();

	m_pFile = nullptr;
}

bool File::open(const char *file, FileAccess::FileAccess access)
{
	auto fileAccess = detail::g_access[access];
	auto r = CreateFileA(file, fileAccess, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	auto error = GetLastError();
	if ((error != 0 && error != ERROR_ALREADY_EXISTS )||
		r == INVALID_HANDLE_VALUE)
	{
		printError(error);

		return false;
	}

	SetLastError(0);
	m_pFile = r;
	return true;
}

bool File::close()
{
	bool result = false;
	if (CloseHandle(m_pFile) != 0)
	{
		m_pFile = nullptr;
		result = true;
	}

	return result;
}

bool File::read(void *ptr, U32 size)
{
	auto result = (ReadFile(m_pFile, ptr, size, nullptr, nullptr) != 0);
	auto error = GetLastError();
	if (error != 0)
	{
		printError(error);
	}

	return result;
}

bool File::write(const void *ptr, U32 size, U32 *pWrittenBytes)
{
	return (WriteFile(m_pFile, ptr, size, (DWORD*)pWrittenBytes, nullptr) != 0);
}

U32 File::getSize() const
{
	LARGE_INTEGER fileSize;
	if (GetFileSizeEx(m_pFile, &fileSize) == 0)
	{
		fileSize.QuadPart = 0;
	}
	
#if _VX_EDITOR
	if (fileSize.QuadPart == 0)
	{
		auto error = GetLastError();
		printError(error);
	}
#endif

	return fileSize.QuadPart;
}