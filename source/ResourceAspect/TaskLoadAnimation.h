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

template<typename T>
class ResourceManager;

namespace vx
{
	struct Animation;
};

#include "TaskLoadFile.h"
#include <vxLib/StringID.h>

struct TaskLoadAnimationDesc
{
	std::string m_fileNameWithPath;
	std::string m_filename;
	Event evt;
	ResourceManager<vx::Animation>* m_animationManager;
	vx::StringID m_sid;
};

class TaskLoadAnimation : public TaskLoadFile
{
	std::string m_filename;
	ResourceManager<vx::Animation>* m_animationManager;
	vx::StringID m_sid;

	TaskReturnType runImpl() override;

public:
	TaskLoadAnimation(TaskLoadAnimationDesc &&desc);
	~TaskLoadAnimation();

	f32 getTimeMs() const override;

	const char* getName(u32* size) const override
	{
		*size = 18;
		return "TaskLoadAnimation";
	}
};