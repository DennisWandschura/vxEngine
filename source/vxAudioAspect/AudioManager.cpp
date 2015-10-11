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
#include <audiopolicy.h>
#include <vxEngineLib/CpuProfiler.h>

namespace Audio
{
	const u32 g_maxAudioSources = 16;

	const u32 g_mask2Channel = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
	const u32 g_mask8Channel = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT;

	namespace AudioManagerCpp
	{
		const __m128 g_channelDirections[8] =
		{
			// front left
			{ 0.707106769f, 0, 0.707106769f, 0 },
			// front right
			{ -0.707106769f, 0, 0.707106769f, 0 },
			// front center
			{ 0, 0, 1, 0 },
			// low frequency
			{ 0, 0, 0, 0 },
			// back left
			{ 0.707106769f, 0, -0.707106769f, 0 },
			// back right
			{ -0.707106769f, 0, -0.707106769f, 0 },
			// side left
			{ 1, 0, 0, 0 },
			// side right
			{ -1, 0, 0, 0 }
		};

		const u8 g_dstChannelIndices[][8]
		{
			// 2 channels
			{ 6, 7, 0, 0, 0, 0, 0, 0 },
			// 6 channels
			{ 6, 7, 0, 0, 0, 0, 0, 0 },
			// 8 channels
			{ 0, 1, 2, 3, 4, 5, 6, 7 },
		};
	}

	struct AudioManager::Entry
	{
		WavFile file;
		WavFormat format;
	};

	AudioManager::AudioManager()
		: m_channelDirections(),
		m_channelIndices(),
		m_dstChannelCount(0),
		m_device(nullptr),
		m_sessionManager(nullptr),
		m_sessionControl(nullptr)
	{
		m_sessionGUID = { 1337, 101, 101, { 0, 0,  0,  0,  0,  0,  0,  0 } };
	}

	bool AudioManager::getWavFormat(void* format)
	{
		IAudioClient* client = nullptr;
		HRESULT hr = m_device->Activate(
			__uuidof(IAudioClient),
			CLSCTX_ALL,
			nullptr,
			(void**)&client);

		SCOPE_EXIT
		{
			client->Release();
		};

		bool result = true;
		WAVEFORMATEXTENSIBLE* closestFormat = nullptr;
		hr = client->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, (WAVEFORMATEX*)format, (WAVEFORMATEX**)&closestFormat);
		if (hr != 0)
		{
			if (closestFormat == nullptr)
			{
				result = false;
			}
			else
			{
				*(WAVEFORMATEXTENSIBLE*)format = *closestFormat;
			}
		}

		return result;
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
		if (hr != S_OK)
			return false;

		hr = pDeviceEnumerator->GetDefaultAudioEndpoint(
			eRender,
			eConsole,
			&m_device);
		if (hr != S_OK)
			return false;

		hr = m_device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr, (void**)&m_sessionManager);
		if (hr != S_OK)
			return false;

		hr = m_sessionManager->GetAudioSessionControl(&m_sessionGUID, 0, &m_sessionControl);

		WAVEFORMATEXTENSIBLE wavFormat;
		wavFormat.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		wavFormat.Format.nChannels = 8;
		wavFormat.Format.nSamplesPerSec = 44100;
		wavFormat.Format.nAvgBytesPerSec = 1411200;
		wavFormat.Format.nBlockAlign = 32;
		wavFormat.Format.wBitsPerSample = 32;
		wavFormat.Format.cbSize = 22;
		wavFormat.Samples.wSamplesPerBlock = 32;
		wavFormat.Samples.wValidBitsPerSample = 32;
		wavFormat.Samples.wReserved = 32;
		wavFormat.dwChannelMask = g_mask8Channel;
		wavFormat.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;

		if (!getWavFormat(&wavFormat))
			return false;

		m_entries = std::make_unique<Audio::WavRenderer[]>(g_maxAudioSources);
		m_entryIndices = std::make_unique<u32[]>(g_maxAudioSources);
		m_freelist.create((u8*)m_entryIndices.get(), g_maxAudioSources, sizeof(u32));

		m_dstBytesPerSample = wavFormat.Format.wBitsPerSample / 8;

		static_assert(sizeof(m_channelIndices) == sizeof(AudioManagerCpp::g_dstChannelIndices[0]), "");
		switch (wavFormat.Format.nChannels)
		{
		case 2:
			memcpy(m_channelIndices, AudioManagerCpp::g_dstChannelIndices[0], sizeof(m_channelIndices));
			break;
		case 6:
			memcpy(m_channelIndices, AudioManagerCpp::g_dstChannelIndices[1], sizeof(m_channelIndices));
			break;
		case 8:
			memcpy(m_channelIndices, AudioManagerCpp::g_dstChannelIndices[2], sizeof(m_channelIndices));
			break;
		default:
			break;
		}

		for (u32 i = 0; i < g_maxAudioSources; ++i)
		{
			IAudioClient* client = nullptr;
			HRESULT hr = m_device->Activate(
				__uuidof(IAudioClient),
				CLSCTX_ALL,
				nullptr,
				(void**)&client);

			s64 devicePeriod;
			REFERENCE_TIME MinimumDevicePeriod = 0;
			hr = client->GetDevicePeriod(&devicePeriod, &MinimumDevicePeriod);

			MinimumDevicePeriod *= 10;

			hr = client->Initialize(
				AUDCLNT_SHAREMODE_SHARED,
				0,
				MinimumDevicePeriod,
				0,
				(WAVEFORMATEX*)&wavFormat,
				&m_sessionGUID);

			m_dstChannelCount = wavFormat.Format.nChannels;

			//CoTaskMemFree(mixFormat);

			u32 bufferFrames = 0;
			hr = client->GetBufferSize(&bufferFrames);

			IAudioRenderClient* renderClient = nullptr;
			hr = client->GetService(
				IID_PPV_ARGS(&renderClient));

			auto hnsActualDuration = (double)MinimumDevicePeriod * bufferFrames / wavFormat.Format.nSamplesPerSec;
			const f32 waitTime = hnsActualDuration / MinimumDevicePeriod / 2.0f;

			Audio::RendererDesc desc;
			desc.audioClient = client;
			desc.audioRenderClient = renderClient;
			desc.bufferFrames = bufferFrames;
			desc.waitTime = waitTime;

			m_entries[i] = Audio::WavRenderer(std::move(desc));
		}

		m_activeEntries[0].reserve(g_maxAudioSources);
		m_activeEntries[1].reserve(g_maxAudioSources);
		m_cleanup.reserve(g_maxAudioSources);

		return true;
	}

	AudioManager::~AudioManager()
	{
		shutdown();
	}

	void AudioManager::shutdown()
	{
		m_activeEntries[0].clear();
		m_activeEntries[1].clear();

		m_entries.reset();
		m_freelist.destroy();
		m_entryIndices.reset();

		if (m_sessionControl)
		{
			m_sessionControl->Release();
			m_sessionControl = nullptr;
		}

		if (m_sessionManager)
		{
			m_sessionManager->Release();
			m_sessionManager = nullptr;
		}

		if (m_device)
		{
			m_device->Release();
			m_device = nullptr;
		}
	}

	void AudioManager::update(f32 dt, const __m128 &listenerPosition, CpuProfiler* profiler)
	{
		profiler->pushMarker("audio update entries");
		for (auto &it : m_activeEntries[0])
		{
			if (!it->eof())
			{
				it->update();
				it->play(m_dstChannelCount, dt, m_channelDirections, listenerPosition);

				m_activeEntries[1].push_back(std::move(it));
			}
			else
			{
				m_cleanup.push_back(it);
			}
		}
		m_activeEntries[0].clear();
		profiler->popMarker();

		m_activeEntries[0].swap(m_activeEntries[1]);

		for (auto &it : m_cleanup)
		{
			it->stop();
			auto index = it - m_entries.get();
			m_freelist.eraseEntry((u8*)&m_entryIndices[index], (u8*)m_entryIndices.get(), sizeof(u32), g_maxAudioSources);
		}
		m_cleanup.clear();
	}

	void AudioManager::updateRotation(const __m128 &listenerQRotation)
	{
		for (u32 chn = 0; chn < m_dstChannelCount; ++chn)
		{
			auto channelIndex = m_channelIndices[chn];
			auto channelDir = AudioManagerCpp::g_channelDirections[channelIndex];

			m_channelDirections[chn] = vx::quaternionRotation(channelDir, listenerQRotation);
		}
	}

	void AudioManager::addAudioFile(const vx::StringID &sid, const AudioFile &file)
	{
		Entry entry;
		entry.format.m_bytesPerSample = file.getBytesPerSample();
		entry.format.m_channels = file.getChannels();
		entry.file = WavFile(file.getData(), file.getSize());

		auto tmp = vx::StringID(sid);
		m_loadedEntries.insert(std::move(tmp), std::move(entry));
	}

	void AudioManager::playSound(const vx::StringID &sid, const vx::float3 &position, const __m128 &listenerPosition, u32* id)
	{
		const auto entry = m_loadedEntries.find(sid);
		if (entry == m_loadedEntries.end())
		{
			*id = 0xffffffff;
			return;
		}

		auto ptr = (u32*)m_freelist.insertEntry((u8*)m_entryIndices.get(), sizeof(u32));
		if (ptr == nullptr)
		{
			*id = 0xffffffff;
			return;
		}

		auto index = ptr - m_entryIndices.get();

		auto renderer = &m_entries[index];
		renderer->initialize(entry->file, entry->format, m_dstBytesPerSample, position, id);

		renderer->start(m_dstChannelCount, m_channelDirections, listenerPosition);
		m_activeEntries[0].push_back(std::move(renderer));

		*id = index;
	}

	void AudioManager::setMasterVolume(f32 volume)
	{
		ISimpleAudioVolume* simpleAudioVolume = nullptr;
		m_sessionManager->GetSimpleAudioVolume(&m_sessionGUID, 0, &simpleAudioVolume);

		simpleAudioVolume->SetMasterVolume(volume, nullptr);
	}

	void AudioManager::setSourcePosition(u32 src, const vx::float3 &position)
	{
		auto renderer = &m_entries[src];
		renderer->setPosition(position);
	}
}