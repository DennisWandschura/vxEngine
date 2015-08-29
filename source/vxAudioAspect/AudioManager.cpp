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

const IID IID_IAudioClient = __uuidof(IAudioClient);

namespace Audio
{
	AudioManager::AudioManager()
		:m_client(nullptr), m_device(nullptr)
	{
	}

	bool AudioManager::initialize()
	{
		IMMDeviceEnumerator* pEnumerator = nullptr;
		const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
		const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
		auto hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator);

		SCOPE_EXIT
		{
			pEnumerator->Release();
		};

		hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &m_device);
		if (hr != 0)
		{
			return false;
		}

		hr = m_device->Activate(IID_IAudioClient, CLSCTX_ALL, nullptr, (void**)&m_client);
		if (hr != 0)
		{
			return false;
		}

		return true;
	}

	AudioManager::~AudioManager()
	{
		shutdown();
	}

	void AudioManager::shutdown()
	{
		if (m_client)
		{
			m_client->Release();
			m_client = nullptr;
		}
		if (m_device)
		{
			m_device->Release();
			m_device = nullptr;
		}
	}
}