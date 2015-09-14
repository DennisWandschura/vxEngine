#include "AudioWavRenderer.h"
#include <algorithm>

namespace Audio
{
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

	WavRenderer::WavRenderer(WavRendererDesc &&desc)
		:Renderer(std::move(desc.rendererDesc)),
		m_wavFile(std::move(desc.m_wavFile)),
		m_format(desc.m_format)
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

	u32 WavRenderer::readBuffer(u8* buffer, u32 frameCount)
	{
		u32 readFrames = 0;
		auto bytesPerSample = m_format.m_bytesPerSample;
		if (bytesPerSample == 2)
		{
			if (m_dstBytes == 2)
			{
				readFrames = m_wavFile.loadDataShort(frameCount, m_format.m_channels, (s16*)buffer, m_dstChannels);
			}
			else if (m_dstBytes == 4)
			{
				readFrames = m_wavFile.loadDataShortToFloat(frameCount, m_format.m_channels, (float*)buffer, m_dstChannels);
			}
		}
		else if (bytesPerSample == 4)
		{
			readFrames = m_wavFile.loadDataFloat(frameCount, m_format.m_channels, (float*)buffer, m_dstChannels);
		}

		return readFrames;
	}

	u32 WavRenderer::eof() const
	{
		return m_wavFile.eof();
	}
}