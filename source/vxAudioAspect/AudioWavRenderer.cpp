#include "AudioWavRenderer.h"
#include <algorithm>

namespace Audio
{
	namespace WavRendererCpp
	{
		u32 loadDataFloat(void* p, u32 frameCount, u32 srcChannels, u8* data, u32 dstChannels, f32 intensity)
		{
			WavFile* ptr = (WavFile*)p;
			return ptr->loadDataFloat(frameCount, srcChannels, data, dstChannels, intensity);
		}

		u32 loadDataShort(void* p, u32 frameCount, u32 srcChannels, u8* data, u32 dstChannels, f32 intensity)
		{
			WavFile* ptr = (WavFile*)p;
			return ptr->loadDataShort(frameCount, srcChannels, data, dstChannels, intensity);
		}

		u32 loadDataShortToFloat(void* p, u32 frameCount, u32 srcChannels, u8* data, u32 dstChannels, f32 intensity)
		{
			WavFile* ptr = (WavFile*)p;
			return ptr->loadDataShortToFloat(frameCount, srcChannels, data, dstChannels, intensity);
		}
	}

	WavRenderer::WavRenderer()
		:Renderer(),
		m_wavFile(),
		m_format()
	{

	}

	WavRenderer::WavRenderer(WavRenderer &&rhs)
		:Renderer(std::move(rhs)),
		m_wavFile(std::move(rhs.m_wavFile)),
		m_format(rhs.m_format)
	{
	}

	WavRenderer::WavRenderer(RendererDesc &&desc)
		:Renderer(std::move(desc)),
		m_wavFile(),
		m_format()
	{

	}

	WavRenderer::~WavRenderer()
	{

	}

	WavRenderer& WavRenderer::operator=(WavRenderer &&rhs)
	{
		Renderer::operator=(std::move(rhs));
		if (this != &rhs)
		{
			m_wavFile = std::move(rhs.m_wavFile);
			std::swap(m_format, rhs.m_format);
		}
		return *this;
	}

	void WavRenderer::initialize(const WavFile &wavFile, const WavFormat &format, const vx::float3 &position)
	{
		m_wavFile = wavFile;
		m_format = format;
		m_position = position;

		auto bytesPerSample = m_format.m_bytesPerSample;
		if (bytesPerSample == 2)
		{
			if (m_dstBytes == 2)
			{
				m_fp = WavRendererCpp::loadDataShort;
			}
			else if (m_dstBytes == 4)
			{
				m_fp = WavRendererCpp::loadDataShortToFloat;
			}
		}
		else if (bytesPerSample == 4)
		{
			m_fp = WavRendererCpp::loadDataFloat;
		}
	}

	u32 WavRenderer::readBuffer(u8* buffer, u32 frameCount, f32 intensity)
	{
		return m_fp(&m_wavFile, frameCount, m_format.m_channels, buffer, m_dstChannels, intensity);
	}

	u32 WavRenderer::eof() const
	{
		return m_wavFile.eof();
	}
}