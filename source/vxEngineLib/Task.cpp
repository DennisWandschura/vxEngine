#include <vxEngineLib/Task.h>
#include <vxEngineLib/TaskAllocator.h>

TaskThreadAllocator* Task::s_allocator{nullptr};

void Task::setAllocator(TaskThreadAllocator* allocator)
{
	s_allocator = allocator;
}

void* Task::operator new(std::size_t size)
{
	return s_allocator->allocate(size);
}

void Task::operator delete(void* p, std::size_t size)
{
	s_allocator->deallocate((u8*)p, size );
}