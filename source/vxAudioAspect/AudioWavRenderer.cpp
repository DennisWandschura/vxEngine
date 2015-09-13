#include "AudioWavRenderer.h"

namespace Audio
{
	WavRenderer::WavRenderer()
		:Renderer(),
		m_wavFile(nullptr),
		m_format()
	{

	}

	WavRenderer::WavRenderer(WavRenderer &&rhs)
		:Renderer(std::move(rhs)),
		m_wavFile(rhs.m_wavFile),
		m_format(rhs.m_format)
	{
		rhs.m_wavFile = nullptr;
	}

	WavRenderer::~WavRenderer()
	{

	}
}