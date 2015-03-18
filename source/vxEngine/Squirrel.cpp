#include "Squirrel.h"
#include <squirrel/squirrel.h>

namespace squirrel
{
	void SquirrelPush::push(HSQUIRRELVM v, const int value)
	{
		sq_pushinteger(v, static_cast<SQInteger>(value));
	}

	void SquirrelPush::push(HSQUIRRELVM v, const SQInteger value)
	{
		sq_pushinteger(v, value);
	}

	void SquirrelPush::push(HSQUIRRELVM v, const float value)
	{
		sq_pushfloat(v, value);
	}

	void SquirrelPush::push(HSQUIRRELVM v, const SQChar *value)
	{
		sq_pushstring(v, value, -1);
	}

	bool SquirrelGet::get(HSQUIRRELVM v, SQInteger idx, int* value)
	{
		bool r = false;
		SQInteger result;
		if (SQ_SUCCEEDED(sq_getinteger(v, idx, &result)))
		{
			*value = result;
			r = true;
		}

		return r;
	}

	bool SquirrelGet::get(HSQUIRRELVM v, SQInteger idx, SQInteger* value)
	{
		bool r = false;
		SQInteger result;
		if (SQ_SUCCEEDED(sq_getinteger(v, idx, &result)))
		{
			*value = result;
			r = true;
		}

		return r;
	}

	bool SquirrelGet::get(HSQUIRRELVM v, SQInteger idx, float* value)
	{
		bool r = false;
		SQFloat result;
		if (SQ_SUCCEEDED(sq_getfloat(v, idx, &result)))
		{
			*value = result;
			r = true;
		}

		return r;
	}

	void SquirrelFunctionWrapper::setup(HSQUIRRELVM v, const SQChar *functionName)
	{
		//auto top = sq_gettop(v); //saves the stack size before the call
		sq_pushroottable(v); //pushes the global table
		sq_pushstring(v, functionName, -1);
	}

	bool SquirrelFunctionWrapper::get(HSQUIRRELVM v)
	{
		return (sq_get(v, -2) >= 0);
	}

	SQInteger SquirrelFunctionWrapper::getTop(HSQUIRRELVM v)
	{
		return sq_gettop(v);
	}

	void SquirrelFunctionWrapper::pushRootTable(HSQUIRRELVM v)
	{
		sq_pushroottable(v);
	}

	void SquirrelFunctionWrapper::pop(HSQUIRRELVM v, SQInteger n)
	{
		sq_pop(v, n);
	}

	void SquirrelFunctionWrapper::call(HSQUIRRELVM v, SQInteger params, SQBool retval, SQBool raiseError)
	{
		sq_call(v, params, retval, raiseError);
	}

	void SquirrelFunctionWrapper::setTop(HSQUIRRELVM v, SQInteger top)
	{
		sq_settop(v, top);
	}
}