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
#include "WavFormat.h"

namespace Audio
{

	struct AudioManager::WavEntry
	{
		IAudioClient* m_client;
		IAudioRenderClient* m_renderClient;
		s64 m_devicePeriod;
		WavFile wav;
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

		/*WAVEFORMATEX wave_format = {};
		wave_format.wFormatTag = WAVE_FORMAT_PCM;
		wave_format.nChannels = 2;
		wave_format.nSamplesPerSec = 44100;
		wave_format.nAvgBytesPerSec = 44100 * 2 * 16 / 8;
		wave_format.nBlockAlign = 2 * 16 / 8;
		const auto wChannels = 2;
		const auto wBitsPerSample = 16;
		const auto blockAlign = wChannels * ((wBitsPerSample + 7) / 8);
		wave_format.wBitsPerSample = 16;*/

	//	m_client->GetMixFormat(&m_format);

		//WAVEFORMATEX* closest;
		//hr = m_client->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, m_format, &closest);

		//AUDCLNT_STREAMFLAGS_EVENTCALLBACK

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
	}

	void AudioManager::update()
	{
		for (auto &it : m_entries2Channel4Byte)
		{

		}
	}

	void AudioManager::addWavFile(WavFile &&wav, const WavFormat &format)
	{
		VX_ASSERT(format.bytesPerSample == 4);
		VX_ASSERT(format.channels == 2);
		VX_ASSERT(format.formatTag == WAVE_FORMAT_IEEE_FLOAT);

		WAVE_FORMAT_FLAC;


		IAudioClient* client = nullptr;
		auto hr = m_device->Activate(
			__uuidof(IAudioClient),
			CLSCTX_ALL,
			nullptr,
			(void**)&client);

		s64 devicePeriod;
		REFERENCE_TIME MinimumDevicePeriod = 0;
		hr = client->GetDevicePeriod(&devicePeriod, &MinimumDevicePeriod);

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

		u32 NumBufferFrames = 0;
		hr = client->GetBufferSize(&NumBufferFrames);

		IAudioRenderClient* renderClient = nullptr;
		hr = client->GetService(
			IID_PPV_ARGS(&renderClient));

		/*u8* buffer = nullptr;
		renderClient->GetBuffer(NumBufferFrames, &buffer);

		auto read_count = wav.readDataFloat28(NumBufferFrames, (float*)buffer);

		hr = renderClient->ReleaseBuffer((UINT32)read_count, 0);

		hr = client->Start();*/

		WavEntry entry;
		entry.m_client = client;
		entry.m_devicePeriod = devicePeriod;
		entry.m_renderClient = renderClient;
		entry.wav = std::move(wav);

		m_entries2Channel4Byte.push_back(std::move(entry));
	}
}