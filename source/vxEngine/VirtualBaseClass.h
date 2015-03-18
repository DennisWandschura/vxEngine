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

	static void registerFunction(U32 classId, U32 funId, FP fun)
	{
		s_vtbl[classId][funId] = fun;
	}

	static void registerClass(U32 classId, const rtti::TypeData *pTypeData)
	{
		s_pTypeData[classId] = pTypeData;
	}

	U32 m_classId;

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

template<U32 totalClasses, U32 totalVirtualMethods>
typename Base<totalClasses, totalVirtualMethods>::FP Base<totalClasses, totalVirtualMethods>::s_vtbl[totalClasses][totalVirtualMethods];

template<U32 totalClasses, U32 totalVirtualMethods>
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