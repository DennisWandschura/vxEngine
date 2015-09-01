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

#include <vxLib/types.h>

namespace d3d
{
	template<typename T>
	class Object
	{
	protected:
		T* m_object;

	public:
		Object() :m_object(nullptr) {}

		Object(const Object&) = delete;

		Object(Object &&rhs)
			:m_object(rhs.m_object)
		{
			rhs.m_object = nullptr;
		}

		~Object()
		{
			destroy();
		}

		Object& operator=(const Object&) = delete;

		Object& operator=(Object &&rhs)
		{
			if (this != &rhs)
			{
				swap(rhs);
			}

			return *this;
		}

		void destroy()
		{
			if (m_object)
			{
				m_object->Release();
				m_object = nullptr;
			}
		}

		void swap(Object &other)
		{
			std::swap(m_object, other.m_object);
		}

		T* operator->() { return m_object; }
		const T* operator->() const { return m_object; }

		T* get() { return m_object; }
		const T* get() const { return m_object; }

		T** getAddressOf() { return &m_object; }
		const T** getAddressOf() const { return &m_object; }
	};
}