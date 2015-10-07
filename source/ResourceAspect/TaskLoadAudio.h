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

class AudioFile;

template<typename T>
class ResourceManager;

#include "TaskLoadFile.h"
#include <vxLib/StringID.h>

struct TaskLoadAudioDesc
{
	std::string m_fileNameWithPath;
	std::string m_fileName;
	ResourceManager<AudioFile>* audioDataManager;
	Event evt;
	vx::StringID m_sid;
};

class TaskLoadAudio : public TaskLoadFile
{
	ResourceManager<AudioFile>* m_audioDataManager;
	vx::StringID m_sid;
	std::string m_fileName;

	TaskReturnType runImpl() override;

public:
	explicit TaskLoadAudio(TaskLoadAudioDesc &&desc);
	~TaskLoadAudio();

	f32 getTimeMs() const override { return 0.2f; }

	const char* getName(u32* size) const override
	{
		*size = 14;
		return "TaskLoadAudio";
	}
};