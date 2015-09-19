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
#include "AudioAspect.h"
#include "WavFile.h"
#include "WavFormat.h"
#include <vxEngineLib/FileEntry.h>
#include <vxLib/Variant.h>
#include <vxEngineLib/Message.h>
#include <vxEngineLib/MessageTypes.h>
#include <vxEngineLib/FileMessage.h>
#include <vxEngineLib/AudioFile.h>
#include <vxEngineLib/AudioMessage.h>

namespace vx
{
	AudioAspect::AudioAspect()
		:m_resourceAspect(nullptr),
		m_masterVolume(1.0f)
	{

	}

	AudioAspect::~AudioAspect()
	{

	}

	bool AudioAspect::initialize(ResourceAspectInterface* resourceAspect)
	{
		if (!m_audioManager.initialize())
			return false;

		m_resourceAspect = resourceAspect;

		vx::FileEntry wavFile("step1.wav", vx::FileType::Audio);
		vx::Variant args;
		args.u64 = 0;

		m_resourceAspect->requestLoadFile(wavFile, args);

		return true;
	}

	void AudioAspect::shutdown()
	{
		m_audioManager.shutdown();
	}

	void AudioAspect::update(f32 dt)
	{
		m_audioManager.update(dt);
	}

	void AudioAspect::onFileMessage(const Message &msg)
	{
		auto type = (vx::FileMessage)msg.code;
		if (type == vx::FileMessage::Audio_Loaded)
		{
			auto sid = vx::StringID(msg.arg1.u64);
			auto file = m_resourceAspect->getAudioFile(sid);

			m_audioManager.addAudioFile(sid, *file);
		}
	}

	void AudioAspect::onAudioMessage(const Message &msg)
	{
		auto type = (vx::AudioMessage)msg.code;
		switch (type)
		{
		case vx::AudioMessage::PlaySound:
		{
			auto sid = vx::StringID(msg.arg1.u64);
			m_audioManager.playSound(sid);
		}break;
		default:
			break;
		}
	}

	void AudioAspect::handleMessage(const Message &msg)
	{
		auto type = msg.type;
		switch (type)
		{
		case vx::MessageType::File:
		{
			onFileMessage(msg);
		}break;
		case vx::MessageType::Audio:
			onAudioMessage(msg);
			break;
		default:
			break;
		}
	}

	void AudioAspect::setMasterVolume(f32 volume)
	{
		volume = std::min(volume, 1.0f);
		volume = std::max(volume, 0.0f);

		m_masterVolume = volume;
		m_audioManager.setMasterVolume(m_masterVolume);
	}
}