#pragma once

#include <vxEngineLib/managed_ptr.h>
#include <vxEngineLib/ArrayAllocator.h>

template<typename T, typename ...Args>
typename std::enable_if<!std::is_array<T>::value, managed_ptr<T>>::type
createPtr(ArrayAllocator* alloc, Args&& ...args)
{
	managed_ptr<T> ptr = alloc->allocate(sizeof(T), __alignof(T));

	if (ptr.get() != nullptr)
	{
		new (ptr.get()) T(std::forward<Args>(args)...);
	}

	return ptr;
}

template<typename T>
typename std::enable_if<std::is_array<T>::value && std::extent<T>::value == 0,
	managed_ptr<T, DefaultDeleter<T>> >::type createPtr(ArrayAllocator* alloc, unsigned int count)
{
	typedef typename std::remove_extent<T>::type _Elem;

	managed_ptr<T, DefaultDeleter<T>> ptr = alloc->allocate(sizeof(_Elem) * count, __alignof(_Elem));

	auto p = ptr.get();
	if (p != nullptr)
	{
		auto last = p + count;
		while (p != last)
		{
			new (p)T{};
			++p;
		}
	}

	return ptr;
}