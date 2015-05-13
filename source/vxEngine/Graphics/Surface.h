#pragma once
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

#include <vxLib/math/Vector.h>
#include <memory>

namespace Graphics
{
	class Surface
	{
		vx::uint3 m_dimension;
		u32 m_size;
		std::unique_ptr<u8[]> m_pixels;

	public:
		Surface();
		Surface(const Surface&) = delete;
		Surface(Surface &&rhs);
		~Surface();

		Surface& operator=(const Surface&) = delete;
		Surface& operator=(Surface &&rhs);

		void create(const vx::uint3 &dimension, u32 size, u8* pixels);

		void clear();

		const vx::uint3& getDimension() const { return m_dimension; }
		u32 getSize() const { return m_size; }

		u8* getPixels() { return m_pixels.get(); }
		const u8* getPixels() const { return m_pixels.get(); }
	};
}