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

#include "context.h"
#include <vxLib/math/Vector.h>

namespace cl
{
	enum class ImageDataType : cl_channel_type
	{
		norm_int8 = CL_SNORM_INT8,
		norm_int16 = CL_SNORM_INT16,
		norm_uint8 = CL_UNORM_INT8,
		norm_uint16 = CL_UNORM_INT16,
		norm_uint24 = CL_UNORM_INT24,
		norm_uint16_565 = CL_UNORM_SHORT_565,
		norm_uint16_555 = CL_UNORM_SHORT_555,
		norm_uint32_101010 = CL_UNORM_INT_101010,
		s8 = CL_SIGNED_INT8,
		s16 = CL_SIGNED_INT16,
		s32 = CL_SIGNED_INT32,
		uint8 = CL_UNSIGNED_INT8,
		uint16 = CL_UNSIGNED_INT16,
		uint32 = CL_UNSIGNED_INT32,
		f16 = CL_HALF_FLOAT,
		f32 = CL_FLOAT,
	};

	enum class ImageChannelOrder : cl_channel_order
	{
		r = CL_R,
		a = CL_A,
		rg = CL_RG,
		ra = CL_RA,
		rgb = CL_RGB,
		rgba = CL_RGBA,
		bgra = CL_BGRA,
		argb = CL_ARGB,
		intensity = CL_INTENSITY,
		luminance = CL_LUMINANCE,
		rx = CL_Rx,
		rgx = CL_RGx,
		rgbx = CL_RGBx,
		depth = CL_DEPTH,
		depth_stencil = CL_DEPTH_STENCIL,
		srgb = CL_sRGB,
		srgbx = CL_sRGBx,
		srgba = CL_sRGBA,
		sbgra = CL_sBGRA,
		abgr = CL_ABGR
	};

	struct ImageFormat
	{
		cl_image_format m_format;

		ImageFormat() :m_format(){}
		ImageFormat(ImageChannelOrder order, ImageDataType dataType)
			:m_format({ (cl_channel_order)order, (cl_channel_type)dataType }){}

		operator cl_image_format&()
		{
			return m_format;
		}

		operator const cl_image_format&() const
		{
			return m_format;
		}

		cl_image_format& get()
		{
			return m_format;
		}

		const cl_image_format& get() const
		{
			return m_format;
		}
	};

	struct ImageDesc
	{
		cl_image_desc m_desc;

		ImageDesc() :m_desc(){}

		operator cl_image_desc&()
		{
			return m_desc;
		}

		operator const cl_image_desc&() const
		{
			return m_desc;
		}

		void set2D(const vx::uint2 &size)
		{
			m_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
			m_desc.image_width = size.x;
			m_desc.image_height = size.y;
			m_desc.image_depth = 0;
			m_desc.image_row_pitch = 0;
			m_desc.image_slice_pitch = 0;
			m_desc.num_samples = 0;
			m_desc.num_mip_levels = 0;
			m_desc.buffer = 0;
		}

		void set3D(const vx::uint3 &size)
		{
			m_desc.image_type = CL_MEM_OBJECT_IMAGE3D;
			m_desc.image_width = size.x;
			m_desc.image_height = size.y;
			m_desc.image_depth = size.z;
			m_desc.image_row_pitch = 0;
			m_desc.image_slice_pitch = 0;
			m_desc.num_samples = 0;
			m_desc.num_mip_levels = 0;
			m_desc.buffer = 0;
		}

		const cl_image_desc& get() const
		{
			return m_desc;
		}
	};

	class Image
	{
		cl_mem m_id;

	public:
		Image() :m_id(nullptr){}
		Image(const Image&) = delete;
		Image(Image &&rhs) :m_id(rhs.m_id){ rhs.m_id = nullptr; }
		~Image(){ destroy(); }

		Image& operator=(const Image&) = delete;

		Image& operator=(Image &&rhs)
		{
			if (this != &rhs)
			{
				auto tmp = m_id;
				m_id = rhs.m_id;
				rhs.m_id = tmp;
			}

			return *this;
		}

		bool create(const Context &context, const ImageFormat &format, const ImageDesc &desc)
		{
			cl_int error = 0;
			if (m_id == nullptr)
			{
				m_id = clCreateImage(context, CL_MEM_READ_ONLY, &format.get(), &desc.get(), nullptr, &error);
			}
			return (error == CL_SUCCESS);
		}

		void destroy()
		{
			if (m_id)
			{
				if (clReleaseMemObject(m_id) == CL_SUCCESS)
					m_id = nullptr;
			}
		}
	};
}