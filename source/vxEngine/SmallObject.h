#pragma once

class SmallObjAllocator;

#include <new>

class SmallObject
{
	static SmallObjAllocator* s_pAllocator;

public:
	virtual ~SmallObject(){}

	static void* operator new(std::size_t size);
	static void operator delete(void* p, std::size_t size);

	static void setAllocator(SmallObjAllocator* pAllocator);
};