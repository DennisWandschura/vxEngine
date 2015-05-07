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

#include <vxLib/types.h>
#include <cstring>

namespace vx
{
	///////////////////////////////// memcpy

	template<typename T>
	inline void memcpy(u8* dst, const T &src)
	{
		::memcpy(dst, (u8*)&src, sizeof(T));
	}

	template<typename T>
	inline void memcpy(T* dst, const T &src)
	{
		::memcpy((u8*)dst, (u8*)&src, sizeof(T));
	}

	template<typename T>
	inline void memcpy(u8* dst, const T* src, u32 count)
	{
		const auto size = sizeof(T) * count;
		::memcpy(dst, (u8*)src, size);
	}

	template<typename T>
	inline void memcpy(T* dst, const T* src, u32 count)
	{
		const auto size = sizeof(T) * count;
		::memcpy((u8*)dst, (u8*)src, size);
	}

	///////////////////////////////// memcpy

	///////////////////////////////// write

	template<typename T>
	// writes data from src to dst and returns a ptr offset by size of src from dst
	inline u8* write(u8* dst, const T &src)
	{
		::memcpy(dst, (u8*)&src, sizeof(T));

		return (dst + sizeof(T));
	}

	template<typename T>
	// writes data from src to dst and returns a ptr offset by size of src from dst
	inline T* write(T* dst, const T &src)
	{
		::memcpy((u8*)dst, (u8*)&src, sizeof(T));

		return (dst + 1);
	}

	template<typename T>
	// writes data from src to dst and returns a ptr offset by size of src from dst
	inline u8* write(u8* dst, const T* src, u32 count)
	{
		const auto size = sizeof(T) * count;
		::memcpy(dst, (u8*)src, size);

		return (dst + size);
	}

	template<typename T>
	// writes data from src to dst and returns a ptr offset by size of src from dst
	inline T* write(T* dst, const T* src, u32 count)
	{
		const auto size = sizeof(T) * count;
		::memcpy((u8*)dst, (u8*)src, size);

		return (dst + count);
	}

	///////////////////////////////// write

	///////////////////////////////// read

	template<typename T>
	// writes data from src to dst and returns a ptr offset by size of dst from src
	inline const u8* read(T& dst, const u8* src)
	{
		::memcpy((u8*)&dst, src, sizeof(T));

		return (src + sizeof(T));
	}

	template<typename T>
	// writes data from src to dst and returns a ptr offset by size of dst from src
	inline const T* read(T& dst, const T* src)
	{
		::memcpy((u8*)&dst, (u8*)src, sizeof(T));

		return (src + 1);
	}

	template<typename T>
	// writes data from src to dst and returns a ptr offset by size of dst from src
	inline const u8* read(T* dst, const u8* src, u32 count)
	{
		auto size = sizeof(T) * count;
		::memcpy((u8*)dst, src, size);

		return (src + size);
	}

	template<typename T>
	// writes data from src to dst and returns a ptr offset by size of dst from src
	inline const T* read(T* dst, const T* src, u32 count)
	{
		auto size = sizeof(T) * count;
		::memcpy((u8*)dst, src, size);

		return (src + count);
	}

	///////////////////////////////// read
}