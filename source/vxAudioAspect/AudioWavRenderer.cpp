#include "AudioWavRenderer.h"
#include <algorithm>

namespace Audio
{
	namespace WavRendererCpp
	{
		u32 loadDataFloat(void* p, u32 bufferFrameCount, u32 srcChannels, u8* data, u8 dstChannelCount, f32 intensity, const __m128(&directions)[8], const __m128 &directionToListener)
		{
			WavFile* ptr = (WavFile*)p;
			return ptr->loadDataFloat(bufferFrameCount, srcChannels, data, dstChannelCount, intensity, directions, directionToListener);
		}

		u32 loadDataShort(void* p, u32 bufferFrameCount, u32 srcChannels, u8* data, u8 dstChannelCount, f32 intensity, const __m128(&directions)[8], const __m128 &directionToListener)
		{
			WavFile* ptr = (WavFile*)p;
			return ptr->loadDataShort(bufferFrameCount, srcChannels, data, dstChannelCount, intensity, directions, directionToListener);
		}

		u32 loadDataShortToFloat(void* p, u32 bufferFrameCount, u32 srcChannels, u8* data, u8 dstChannelCount, f32 intensity, const __m128(&directions)[8], const __m128 &directionToListener)
		{
			WavFile* ptr = (WavFile*)p;
			return ptr->loadDataShortToFloat(bufferFrameCount, srcChannels, data, dstChannelCount, intensity, directions, directionToListener);
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

	void WavRenderer::initialize(const WavFile &wavFile, const WavFormat &format, u32 dstBytesPerSample, const vx::float3 &position, u32* id)
	{
		m_wavFile = wavFile;
		m_format = format;
		m_position = vx::loadFloat3(&position);
		m_id = id;

		auto bytesPerSample = m_format.m_bytesPerSample;
		if (bytesPerSample == 2)
		{
			if (dstBytesPerSample == 2)
			{
				m_fp = WavRendererCpp::loadDataShort;
			}
			else if (dstBytesPerSample == 4)
			{
				m_fp = WavRendererCpp::loadDataShortToFloat;
			}
		}
		else if (bytesPerSample == 4)
		{
			m_fp = WavRendererCpp::loadDataFloat;
		}
	}

	u32 WavRenderer::readBuffer(u8* buffer, u32 frameCount, u8 dstChannnelCount, f32 intensity, const __m128(&directios)[8], const __m128 &directionToListener)
	{
		return m_fp(&m_wavFile, frameCount, m_format.m_channels, buffer, dstChannnelCount, intensity, directios, directionToListener);
	}

	u32 WavRenderer::eof() const
	{
		return m_wavFile.eof();
	}
}