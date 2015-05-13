#include "Surface.h"
#include <algorithm>

namespace Graphics
{
	Surface::Surface()
		: m_dimension(0, 0, 0),
		m_size(0),
		m_pixels(nullptr)
	{
	}

	Surface::Surface(Surface &&rhs)
		:m_dimension(rhs.m_dimension),
		m_size(rhs.m_size),
		m_pixels(std::move(rhs.m_pixels))
	{
	}

	Surface::~Surface()
	{
		clear();
	}

	Surface& Surface::operator = (Surface &&rhs)
	{
		if (this != &rhs)
		{
			std::swap(m_dimension, rhs.m_dimension);
			std::swap(m_size, rhs.m_size);
			std::swap(m_pixels, rhs.m_pixels);
		}
		return *this;
	}

	void Surface::create(const vx::uint3 &dimension, u32 imgsize, u8* pixels)
	{
		VX_ASSERT(m_dimension.x != 0);
		VX_ASSERT(m_dimension.y != 0);
		VX_ASSERT(m_dimension.z != 0);
		VX_ASSERT(imgsize != 0);
		VX_ASSERT(pixels);

		clear();

		m_dimension = dimension;
		m_size = imgsize;
		m_pixels = std::make_unique<u8[]>(imgsize);
		::memcpy(m_pixels.get(), pixels, imgsize);
	}

	void Surface::clear()
	{
		m_pixels.reset();
		m_size = 0;
	}
}