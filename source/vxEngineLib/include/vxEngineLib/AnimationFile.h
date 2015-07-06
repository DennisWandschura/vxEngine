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

#include <vxEngineLib/Serializable.h>
#include <vxEngineLib/Animation.h>

namespace vx
{
	class AnimationFile : public Serializable
	{
		Animation m_animation;
		char m_name[32];

	public:
		explicit AnimationFile(u32 version);
		AnimationFile(const AnimationFile&) = delete;
		AnimationFile(AnimationFile &&rhs);
		AnimationFile(u32 version, Animation &&animation, const char* name);
		~AnimationFile();

		AnimationFile& operator=(const AnimationFile&) = delete;
		AnimationFile& operator=(AnimationFile &&rhs);

		void saveToFile(File* f) const override;

		const u8* loadFromMemory(const u8 *ptr, u32 size, vx::Allocator* allocator) override;

		u64 getCrc() const override;
		static u32 getGlobalVersion();
	};
}