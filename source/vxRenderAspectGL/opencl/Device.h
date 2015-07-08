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

#include "platform.h"

namespace cl
{
	enum class DeviceType : cl_device_type
	{
		Cpu = CL_DEVICE_TYPE_CPU,
		Gpu = CL_DEVICE_TYPE_GPU
	};

	enum class DeviceInfo : cl_device_info
	{
		Address_Bits = CL_DEVICE_ADDRESS_BITS,
		Endian = CL_DEVICE_ENDIAN_LITTLE,
		Extensions = CL_DEVICE_EXTENSIONS,
		Global_Mem_Cache_Size = CL_DEVICE_GLOBAL_MEM_CACHE_SIZE,
		Device_Mem_Cacheline_Size = CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE,
		Device_Mem_Cache_Size = CL_DEVICE_GLOBAL_MEM_SIZE,
		Name = CL_DEVICE_NAME,
		Max_Compute_Units = CL_DEVICE_MAX_COMPUTE_UNITS
	};

	class Device
	{
		cl_device_id m_id;

	public:
		Device() :m_id(){}
		Device(const Device&) = delete;
		Device(Device &&rhs) :m_id(rhs.m_id){ rhs.m_id = 0; }
		~Device(){}

		Device& operator=(const Device&) = delete;

		Device& operator=(Device &&rhs)
		{
			if (this != &rhs)
			{
				auto tmp = m_id;
				m_id = rhs.m_id;
				rhs.m_id = tmp;
			}

			return *this;
		}

		static cl_uint getDeviceCount(const Platform &platform, DeviceType deviceType)
		{
			cl_uint count = 0;

			clGetDeviceIDs(platform, (cl_device_type)deviceType, 0, nullptr, &count);

			return count;
		}

		static void getDevices(const Platform &platform, DeviceType deviceType, cl_uint count, Device* devices)
		{
			clGetDeviceIDs(platform, (cl_device_type)deviceType, count, (cl_device_id*)devices, nullptr);
		}

		template<DeviceInfo INFO, typename = std::enable_if <
			INFO == DeviceInfo::Address_Bits ||
			INFO == DeviceInfo::Device_Mem_Cacheline_Size ||
			INFO == DeviceInfo::Max_Compute_Units
		> ::type>
		cl_uint getInfo()
		{
			cl_uint value = 0;
			clGetDeviceInfo(m_id, (cl_device_info)INFO, sizeof(cl_uint), &value, nullptr);
			return value;
		}

		template<DeviceInfo INFO, typename = std::enable_if<
			INFO == DeviceInfo::Endian ||
			INFO == DeviceInfo::Name ||
			INFO == DeviceInfo::Extensions
		>::type>
		std::string getInfo()
		{
			size_t size = 0;
			clGetDeviceInfo(m_id, (cl_device_info)INFO, 0, nullptr, &size);

			std::string value(size, '\0');

			clGetDeviceInfo(m_id, (cl_device_info)INFO, size, &value[0], nullptr);

			return value;
		}

		template<DeviceInfo INFO, typename = std::enable_if<
			INFO == DeviceInfo::Global_Mem_Cache_Size ||
			INFO == DeviceInfo::Device_Mem_Cache_Size
		>::type>
		cl_ulong getInfo()
		{
			cl_ulong value = 0;
			clGetDeviceInfo(m_id, (cl_device_info)INFO, sizeof(cl_ulong), &value, nullptr);

			return value;
		}
	};
}