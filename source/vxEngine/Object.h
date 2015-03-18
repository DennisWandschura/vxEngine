#pragma once

#include "rtti.h"

class ObjectBase
{
public:
	virtual ~ObjectBase(){}
};

template<class T>
class Object : public T
{
	static rtti::TypeData *s_typeData;
};