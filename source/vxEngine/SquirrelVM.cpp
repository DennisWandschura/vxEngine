#include "SquirrelVM.h"
#include <squirrel/squirrel.h>
#include <squirrel/sqstdaux.h>
#include <squirrel/sqstdio.h> 
#include <stdio.h>
#include <stdarg.h>

#ifdef SQUNICODE 
#define scvprintf vwprintf 
#else 
#define scvprintf vprintf 
#endif 

namespace
{
	void printfunc(HSQUIRRELVM v, const SQChar *s, ...)
	{
		//VX_UNREFERENCED_PARAMETER(v);

		va_list arglist;
		va_start(arglist, s);
		scvprintf(s, arglist);
		va_end(arglist);
	}

	SQInteger register_global_func(HSQUIRRELVM v, SQFUNCTION f, const wchar_t *fname)
	{
		sq_pushroottable(v);
		sq_pushstring(v, fname, -1);
		sq_newclosure(v, f, 0); //create a new function
		sq_createslot(v, -3);
		sq_pop(v, 1); //pops the root table
		return 0;
	}
}

SquirrelVM::SquirrelVM()
{

}

SquirrelVM::~SquirrelVM()
{
	if (m_vm)
	{
		sq_pop(m_vm, 1);
		sq_close(m_vm);
	}
}

void SquirrelVM::initialize()
{
	if (!m_vm)
	{
		m_vm = sq_open(1024);

		sqstd_seterrorhandlers(m_vm);
		sq_setprintfunc(m_vm, printfunc, nullptr); //sets the print function
		sq_pushroottable(m_vm);
	}
}

void SquirrelVM::registerGlobalFunction(SQFUNCTION f, const SQChar* name)
{
	register_global_func(m_vm, f, name);
}

bool SquirrelVM::doFile(const SQChar* file)
{
	return (sqstd_dofile(m_vm, file, 0, 1) >= 0);
}

bool SquirrelVM::loadFile(const SQChar* file)
{
	return (sqstd_loadfile(m_vm, file, 1) >= 0);
}