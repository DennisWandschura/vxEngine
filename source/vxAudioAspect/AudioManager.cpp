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

namespace Audio
{
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

		m_entries.clear();

		for (auto &it : m_wavFiles)
		{
			delete(it);
		}
		m_wavFiles.clear();
	}

	void AudioManager::update(f32 dt)
	{
		for (auto &it : m_entries)
		{
			if (!it.eof())
			{
				it.update();
				it.play(dt);
			}
		}
	}

	void AudioManager::addWavFile(WavFile* wav, const WavFormat &format)
	{
		m_wavFiles.push_back(wav);

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

		/*u8* buffer = nullptr;
		renderClient->GetBuffer(NumBufferFrames, &buffer);

		auto read_count = wav.readDataFloat28(NumBufferFrames, (float*)buffer);

		hr = renderClient->ReleaseBuffer((UINT32)read_count, 0);

		hr = client->Start();*/

		auto hnsActualDuration = (double)MinimumDevicePeriod * bufferFrames / mixFormat->nSamplesPerSec;
		const f32 waitTime = hnsActualDuration / MinimumDevicePeriod / 2.0f;

		Audio::WavRenderer renderer;
		renderer.setFile(wav, format);
		renderer.setDestinationFormat(bufferFrames, mixFormat->nChannels, mixFormat->wBitsPerSample / 8, renderClient, client, waitTime);

		BYTE *pData = nullptr;
		hr = renderClient->GetBuffer(bufferFrames, &pData);

		auto read_count = renderer.readBuffer(pData, bufferFrames);

		hr = renderClient->ReleaseBuffer((UINT32)read_count, 0);

		hr = client->Start();

		m_entries.push_back(std::move(renderer));
	}
}