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

#include "rtti.h"

template<unsigned totalClasses, unsigned totalVirtualMethods>
class Base
{
public:
	typedef void(Base::*FP)();
	typedef void(Base::*FPCall)(const Base &);

	static FP s_vtbl[totalClasses][totalVirtualMethods];
	static const rtti::TypeData *s_pTypeData [totalClasses];

protected:
	static unsigned getClassId()
	{
		static unsigned id = 0;
		assert(id < totalClasses);
		return id++;
	}

	static void registerFunction(u32 classId, u32 funId, FP fun)
	{
		s_vtbl[classId][funId] = fun;
	}

	static void registerClass(u32 classId, const rtti::TypeData *pTypeData)
	{
		s_pTypeData[classId] = pTypeData;
	}

	u32 m_classId;

public:
	Base(unsigned classId) :m_classId(classId){}

	~Base()
	{
		rtti::Manager::destroy(s_pTypeData[m_classId], this);
	}

	// virtual 
	void test()
	{
		(this->*s_vtbl[m_classId][1])();
	}

	void doit()
	{
		(this->*s_vtbl[m_classId][2])();
	}
};

template<u32 totalClasses, u32 totalVirtualMethods>
typename Base<totalClasses, totalVirtualMethods>::FP Base<totalClasses, totalVirtualMethods>::s_vtbl[totalClasses][totalVirtualMethods];

template<u32 totalClasses, u32 totalVirtualMethods>
const rtti::TypeData* Base<totalClasses, totalVirtualMethods>::s_pTypeData[totalClasses];

template<class T>
class BaseWrapper : public Base<10, 10>
{
	static unsigned s_classId;
	static bool s_registered;

protected:
	static bool registerClass()
	{
		assert(&T::test != &Base::test);
		assert(&T::doit != &Base::doit);

		Base::registerClass(s_classId, rtti::TypeCreator<T>::getTypeData());
		Base::registerFunction(s_classId, 1, (Base::FP)&T::test);
		Base::registerFunction(s_classId, 2, (Base::FP)&T::doit);

		return true;
	}

public:
	BaseWrapper() : Base(s_classId){ assert(s_registered == 1); }
};

template<class T>
unsigned BaseWrapper<T>::s_classId = Base::getClassId();

template<class T>
bool BaseWrapper<T>::s_registered = BaseWrapper<T>::registerClass();