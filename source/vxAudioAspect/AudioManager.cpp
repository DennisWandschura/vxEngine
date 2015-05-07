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
#include "vxAudio/AudioManager.h"
#include <AL/alc.h>
#include <cstdio>

#pragma comment(lib, "OpenAL32.lib")

AudioManager::AudioManager()
{
}

bool AudioManager::init()
{
	m_pDevice = alcOpenDevice(nullptr);
	if (m_pDevice == nullptr)
		return false;

	m_pContext = alcCreateContext(m_pDevice, nullptr);
	if (!alcMakeContextCurrent(m_pContext))
	{
		//puts(" failed to make context current");
		return false;
	}

	//puts("Created audio device and context");
	return true;
}

AudioManager::~AudioManager()
{
	shutdown();
}

void AudioManager::shutdown()
{
	if (m_pContext)
	{
		alcMakeContextCurrent(nullptr);
		alcDestroyContext(m_pContext);
		m_pContext = nullptr;
	}

	if (m_pDevice)
	{
		alcCloseDevice(m_pDevice);
		m_pDevice = nullptr;
	}
}