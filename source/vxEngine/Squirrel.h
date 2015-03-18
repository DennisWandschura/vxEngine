#pragma once

#include <vxLib/types.h>

struct SQVM;

typedef long long SQInteger;
typedef unsigned long long SQBool;
typedef SQVM* HSQUIRRELVM;
#if _UNICODE
typedef wchar_t SQChar;
#else
typedef char SQChar;
#endif

namespace squirrel
{
	struct SquirrelPush
	{
		static void push(HSQUIRRELVM v, const int value);
		static void push(HSQUIRRELVM v, const SQInteger value);
		static void push(HSQUIRRELVM v, const float value);
		static void push(HSQUIRRELVM v, const SQChar *value);
	};

	struct SquirrelGet
	{
		static bool get(HSQUIRRELVM v, SQInteger idx, int* value);
		static bool get(HSQUIRRELVM v, SQInteger idx, SQInteger* value);
		static bool get(HSQUIRRELVM v, SQInteger idx, float* value);
	};

	template<typename ...Args>
	struct Push_Args;

	template<>
	struct Push_Args<>
	{
		enum{ size = 0 };

		static void push(HSQUIRRELVM v)
		{
			VX_UNREFERENCED_PARAMETER(v);
		}
	};

	template<typename T1>
	struct Push_Args<T1>
	{
		enum{ size = 1 };

		static void push(HSQUIRRELVM v, const T1 t1)
		{
			SquirrelPush::push(v, t1);
		}
	};

	template<typename T1, typename T2>
	struct Push_Args<T1, T2>
	{
		enum{ size = 2 };

		static void push(HSQUIRRELVM v, const T1 t1, const T2 t2)
		{
			SquirrelPush::push(v, t1);
			SquirrelPush::push(v, t2);
		}
	};

	template<typename T1, typename T2, typename T3>
	struct Push_Args<T1, T2, T3>
	{
		enum{ size = 3 };

		static void push(HSQUIRRELVM v, const T1 t1, const T2 t2, const T3 t3)
		{
			SquirrelPush::push(v, t1);
			SquirrelPush::push(v, t2);
			SquirrelPush::push(v, t3);
		}
	};

	template<typename T1, typename T2, typename T3, typename T4>
	struct Push_Args<T1, T2, T3, T4>
	{
		enum{ size = 4 };

		static void push(HSQUIRRELVM v, const T1 t1, const T2 t2, const T3 t3, const T4 t4)
		{
			SquirrelPush::push(v, t1);
			SquirrelPush::push(v, t2);
			SquirrelPush::push(v, t3);
			SquirrelPush::push(v, t4);
		}
	};

	struct SquirrelFunctionWrapper
	{
	protected:
		static void setup(HSQUIRRELVM v, const SQChar *functionName);
		static bool get(HSQUIRRELVM v);
		static SQInteger getTop(HSQUIRRELVM v);
		static void pushRootTable(HSQUIRRELVM v);
		static void call(HSQUIRRELVM v, SQInteger params, SQBool retval, SQBool raiseError);
		static void pop(HSQUIRRELVM v, SQInteger n);
		static void setTop(HSQUIRRELVM v, SQInteger top);
	};
}

template<typename Return_Type, typename ...Args>
class SquirrelFunction : public squirrel::SquirrelFunctionWrapper
{
	static_assert(std::is_same<Return_Type, int>::value || 
		std::is_same<Return_Type, SQInteger>::value ||
		std::is_same<Return_Type, float>::value, "Wrong return type, only Integer and Float supported !");

	template<typename ...Vargs>
	static bool call(HSQUIRRELVM v, const SQChar *functionName, Return_Type *result, Vargs&& ...args)
	{
		bool succeded = false;

		squirrel::SquirrelFunctionWrapper::setup(v, functionName);
		if (get(v))
		{
			pushRootTable(v);
			squirrel::Push_Args<Args...>::push(v, std::forward<Vargs>(args)...);
			squirrel::SquirrelFunctionWrapper::call(v, 1 + squirrel::Push_Args<Args...>::size, 1, 0);
			succeded = squirrel::SquirrelGet::get(v, getTop(v), result);
		}
		pop(v, 2);

		return succeded;
	}

public:
	bool operator()(HSQUIRRELVM v, const SQChar *functionName, Return_Type *result, Args ...args)
	{
		return call(v, functionName, result, std::forward<Args>(args)...);
	}
};

template<typename ...Args>
class SquirrelFunction<void, Args...> : public squirrel::SquirrelFunctionWrapper
{
	template<typename ...Vargs>
	static void call(HSQUIRRELVM v, const SQChar *functionName, Vargs&& ...args)
	{
		squirrel::SquirrelFunctionWrapper::setup(v, functionName);
		if (get(v))
		{
			pushRootTable(v);
			squirrel::Push_Args<Args...>::push(v, std::forward<Vargs>(args)...);
			squirrel::SquirrelFunctionWrapper::call(v, 1 + squirrel::Push_Args<Args...>::size, 0, 0);
		}
		pop(v, 2);
	}

public:
	void operator()(HSQUIRRELVM v, const SQChar *functionName, Args ...args)
	{
		call(v, functionName, std::forward<Args>(args)...);
	}
};