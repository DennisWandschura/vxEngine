#pragma once
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

struct IMMDevice;
class WavFile;
struct WavFormat;
class AudioFile;

#include <vxLib/types.h>
#include "AudioWavRenderer.h"
#include <vxLib/Container/sorted_vector.h>
#include <vxLib/StringID.h>
#include <vector>

namespace Audio
{
	class AudioManager
	{
		struct Entry;

		std::vector<Audio::WavRenderer> m_activeEntries;
		std::vector<Audio::WavRenderer> m_activeEntries1;
		vx::sorted_vector<vx::StringID, Entry> m_inactiveEntries;

		IMMDevice* m_device;

	public:
		AudioManager();
		~AudioManager();

		bool initialize();
		void shutdown();

		void update(f32 dt);

		void addAudioFile(const vx::StringID &sid, const AudioFile &file);

		void playSound(const vx::StringID &sid);
	};
}