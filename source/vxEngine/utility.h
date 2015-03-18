#pragma once

#include <vxLib/types.h>
#include <cstring>

namespace vx
{
	///////////////////////////////// memcpy

	template<typename T>
	inline void memcpy(U8* dst, const T &src)
	{
		::memcpy(dst, (U8*)&src, sizeof(T));
	}

	template<typename T>
	inline void memcpy(T* dst, const T &src)
	{
		::memcpy((U8*)dst, (U8*)&src, sizeof(T));
	}

	template<typename T>
	inline void memcpy(U8* dst, const T* src, U32 count)
	{
		const auto size = sizeof(T) * count;
		::memcpy(dst, (U8*)src, size);
	}

	template<typename T>
	inline void memcpy(T* dst, const T* src, U32 count)
	{
		const auto size = sizeof(T) * count;
		::memcpy((U8*)dst, (U8*)src, size);
	}

	///////////////////////////////// memcpy

	///////////////////////////////// write

	template<typename T>
	// writes data from src to dst and returns a ptr offset by size of src from dst
	inline U8* write(U8* dst, const T &src)
	{
		::memcpy(dst, (U8*)&src, sizeof(T));

		return (dst + sizeof(T));
	}

	template<typename T>
	// writes data from src to dst and returns a ptr offset by size of src from dst
	inline T* write(T* dst, const T &src)
	{
		::memcpy((U8*)dst, (U8*)&src, sizeof(T));

		return (dst + 1);
	}

	template<typename T>
	// writes data from src to dst and returns a ptr offset by size of src from dst
	inline U8* write(U8* dst, const T* src, U32 count)
	{
		const auto size = sizeof(T) * count;
		::memcpy(dst, (U8*)src, size);

		return (dst + size);
	}

	template<typename T>
	// writes data from src to dst and returns a ptr offset by size of src from dst
	inline T* write(T* dst, const T* src, U32 count)
	{
		const auto size = sizeof(T) * count;
		::memcpy((U8*)dst, (U8*)src, size);

		return (dst + count);
	}

	///////////////////////////////// write

	///////////////////////////////// read

	template<typename T>
	// writes data from src to dst and returns a ptr offset by size of dst from src
	inline const U8* read(T& dst, const U8* src)
	{
		::memcpy((U8*)&dst, src, sizeof(T));

		return (src + sizeof(T));
	}

	template<typename T>
	// writes data from src to dst and returns a ptr offset by size of dst from src
	inline const T* read(T& dst, const T* src)
	{
		::memcpy((U8*)&dst, (U8*)src, sizeof(T));

		return (src + 1);
	}

	template<typename T>
	// writes data from src to dst and returns a ptr offset by size of dst from src
	inline const U8* read(T* dst, const U8* src, U32 count)
	{
		auto size = sizeof(T) * count;
		::memcpy((U8*)dst, src, size);

		return (src + size);
	}

	template<typename T>
	// writes data from src to dst and returns a ptr offset by size of dst from src
	inline const T* read(T* dst, const T* src, U32 count)
	{
		auto size = sizeof(T) * count;
		::memcpy((U8*)dst, src, size);

		return (src + count);
	}

	///////////////////////////////// read
}