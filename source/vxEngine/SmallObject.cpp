#include "SmallObject.h"
#include "SmallObjAllocator.h"

SmallObjAllocator* SmallObject::s_pAllocator{ nullptr };

void* SmallObject::operator new(std::size_t size)
{
	return s_pAllocator->allocate(size);
}

void SmallObject::operator delete(void* p, std::size_t size)
{
	s_pAllocator->deallocate((U8*)p, size);
}

void SmallObject::setAllocator(SmallObjAllocator* pAllocator)
{
	s_pAllocator = pAllocator;
}