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

#include <CL/cl.h>
#include <string>

namespace cl
{
	enum class PlatformInfo : cl_platform_info
	{
		Profile = CL_PLATFORM_PROFILE,
		Version = CL_PLATFORM_VERSION,
		Name = CL_PLATFORM_NAME,
		Vendor = CL_PLATFORM_VENDOR,
		Extensions = CL_PLATFORM_EXTENSIONS
	};

	class Platform
	{
		cl_platform_id m_id;

	public:
		Platform():m_id(){}
		Platform(const Platform&) = delete;
		Platform(Platform &&rhs) :m_id(rhs.m_id){ rhs.m_id = 0; }
		~Platform(){}

		Platform& operator=(const Platform&) = delete;

		Platform& operator=(Platform &&rhs)
		{
			if (this != &rhs)
			{
				auto tmp = m_id;
				m_id = rhs.m_id;
				rhs.m_id = tmp;
			}

			return *this;
		}

		static void getPlatforms(cl_uint count, Platform* platforms)
		{
			clGetPlatformIDs(count, (cl_platform_id*)platforms, nullptr);
		}

		static cl_uint getPlatformCount()
		{
			cl_uint count = 0;
			clGetPlatformIDs(0, nullptr, &count);
			return count;
		}

		std::string getInfo(PlatformInfo info)
		{
			size_t size = 0;
			clGetPlatformInfo(m_id, (cl_platform_info)info, 0, nullptr, &size);

			std::string str(size, '\0');

			clGetPlatformInfo(m_id, (cl_platform_info)info, size, &str[0], 0);

			return str;
		}

		operator cl_platform_id&()
		{
			return m_id;
		}

		operator const cl_platform_id&() const
		{
			return m_id;
		}

		cl_platform_id get() const
		{
			return m_id;
		}
	};
}