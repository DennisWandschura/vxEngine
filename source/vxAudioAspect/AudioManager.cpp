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
#include "AudioManager.h"
#include <cstdio>
#include <math.h>
#include <mmdeviceapi.h>
#include <vxLib/ScopeGuard.h>
#include <Audioclient.h>
#include "WavFile.h"
#include <vxEngineLib/AudioFile.h>

namespace Audio
{
	struct AudioManager::Entry
	{
		WavFile file;
		WavFormat format;
	};

	AudioManager::AudioManager()
		:m_device(nullptr)
	{
	}

	bool AudioManager::initialize()
	{
		auto hr = CoInitialize(nullptr);

		IMMDeviceEnumerator* pDeviceEnumerator = nullptr;

		hr = CoCreateInstance(
			__uuidof(MMDeviceEnumerator),
			nullptr,
			CLSCTX_ALL,
			__uuidof(IMMDeviceEnumerator),
			(void**)&pDeviceEnumerator);

		hr = pDeviceEnumerator->GetDefaultAudioEndpoint(
			eRender,
			eConsole,
			&m_device);

		WAVEFORMATEX format;
		format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
		format.nChannels = 8;
		format.nSamplesPerSec = 44100;
		format.nAvgBytesPerSec = 1411200;
		format.nBlockAlign = 32;
		format.wBitsPerSample = 32;
		format.cbSize = 22;

		return true;
	}

	AudioManager::~AudioManager()
	{
		shutdown();
	}

	void AudioManager::shutdown()
	{
		if (m_device)
		{
			m_device->Release();
			m_device = nullptr;
		}

		m_activeEntries.clear();
		m_inactiveEntries.clear();
	}

	void AudioManager::update(f32 dt)
	{
		for (auto &it : m_activeEntries)
		{
			if (!it.eof())
			{
				it.update();
				it.play(dt);

				m_activeEntries1.push_back(std::move(it));
			}
		}
		m_activeEntries.clear();

		m_activeEntries.swap(m_activeEntries1);
	}

	void AudioManager::addAudioFile(const vx::StringID &sid, const AudioFile &file)
	{
		Entry entry;
		entry.format.m_bytesPerSample = file.getBytesPerSample();
		entry.format.m_channels = file.getChannels();
		entry.file = WavFile(file.getData(), file.getSize());

		auto tmp = vx::StringID(sid);
		m_inactiveEntries.insert(std::move(tmp), std::move(entry));
	}

	void AudioManager::playSound(const vx::StringID &sid)
	{
		const auto entry = m_inactiveEntries.find(sid);
		if (entry == m_inactiveEntries.end())
			return;

		IAudioClient* client = nullptr;
		auto hr = m_device->Activate(
			__uuidof(IAudioClient),
			CLSCTX_ALL,
			nullptr,
			(void**)&client);

		s64 devicePeriod;
		REFERENCE_TIME MinimumDevicePeriod = 0;
		hr = client->GetDevicePeriod(&devicePeriod, &MinimumDevicePeriod);

		MinimumDevicePeriod *= 10;

		WAVEFORMATEX* mixFormat;
		client->GetMixFormat(&mixFormat);

		hr = client->Initialize(
			AUDCLNT_SHAREMODE_SHARED,
			0,
			MinimumDevicePeriod,
			0,
			mixFormat,
			nullptr);

		VX_ASSERT(hr == 0);

		u32 bufferFrames = 0;
		hr = client->GetBufferSize(&bufferFrames);

		IAudioRenderClient* renderClient = nullptr;
		hr = client->GetService(
			IID_PPV_ARGS(&renderClient));

		auto hnsActualDuration = (double)MinimumDevicePeriod * bufferFrames / mixFormat->nSamplesPerSec;
		const f32 waitTime = hnsActualDuration / MinimumDevicePeriod / 2.0f;

		Audio::WavRendererDesc desc;
		desc.rendererDesc.audioClient = client;
		desc.rendererDesc.audioRenderClient = renderClient;
		desc.rendererDesc.bufferFrames = bufferFrames;
		desc.rendererDesc.dstBytes = mixFormat->wBitsPerSample / 8;
		desc.rendererDesc.dstChannels = mixFormat->nChannels;
		desc.rendererDesc.waitTime = waitTime;
		desc.m_format = entry->format;
		desc.m_wavFile = entry->file;

		Audio::WavRenderer renderer(std::move(desc));

		renderer.startPlay();
		m_activeEntries.push_back(std::move(renderer));
	}
}