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

#include <CL/opencl.h>

namespace cl
{
	class Device;

	class Context
	{
		cl_context m_context;

	public:
		Context() :m_context(nullptr){}
		Context(const Context&) = delete;
		Context(Context &&rhs) :m_context(rhs.m_context){ rhs.m_context = 0; }
		~Context(){ destroy(); }

		Context& operator=(const Context&) = delete;

		Context& operator=(Context &&rhs)
		{
			if (this != &rhs)
			{
				auto tmp = m_context;
				m_context = rhs.m_context;
				rhs.m_context = tmp;
			}

			return *this;
		}

		operator cl_context&()
		{
			return m_context;
		}

		operator const cl_context&() const
		{
			return m_context;
		}

		cl_int create(const cl_context_properties *properties, cl_uint deviceCount, const Device* devices)
		{
			cl_int error = CL_SUCCESS;
			if (m_context == nullptr)
			{
				m_context = clCreateContext(properties, deviceCount, (cl_device_id*)devices, nullptr, nullptr, &error);
			}
			return error;
		}

		void destroy()
		{
			if (m_context)
			{
				if (clReleaseContext(m_context) == CL_SUCCESS)
					m_context = nullptr;
			}
		}
	};
}