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

#include <vxEngineLib/AnimationFile.h>
#include <vxLib/util/CityHash.h>
#include <vxLib/File/File.h>
#include <vxEngineLib/memcpy.h>

namespace vx
{
	AnimationFile::AnimationFile(u32 version)
		:Serializable(version),
		m_animation(),
		m_name()
	{

	}

	AnimationFile::AnimationFile(u32 version, Animation &&animation, const char* name)
		:Serializable(version),
		m_animation(std::move(animation)),
		m_name()
	{
		strncpy(m_name, name, sizeof(m_name));
	}

	AnimationFile::AnimationFile(AnimationFile &&rhs)
		:Serializable(std::move(rhs)),
		m_animation(std::move(rhs.m_animation)),
		m_name()
	{
		strncpy(m_name, rhs.m_name, sizeof(m_name));
	}

	AnimationFile::~AnimationFile()
	{

	}

	AnimationFile& AnimationFile::operator = (AnimationFile &&rhs)
	{
		if (this != &rhs)
		{
			Serializable::operator=(std::move(rhs));
			std::swap(m_animation, rhs.m_animation);

			const auto bufferSize = sizeof(m_name);
			char buffer[bufferSize];
			strncpy(buffer, rhs.m_name, bufferSize);

			strncpy(rhs.m_name, m_name, bufferSize);
			strncpy(m_name, buffer, bufferSize);
			
		}
		return *this;
	}

	void AnimationFile::saveToFile(File* f) const
	{
		u32 writtenSize = 0;
		m_animation.saveToFile(f, &writtenSize);
		f->write(m_name);

		writtenSize += sizeof(m_name);

		//printf("name: %s\n", m_name);
		//printf("AnimationFile::saveToFile: %u\n", writtenSize);
	}

	const u8* AnimationFile::loadFromMemory(const u8 *ptr, u32 size, vx::Allocator* allocator)
	{
		auto start = ptr;

		ptr = m_animation.loadFromMemory(ptr);
		ptr = vx::read(m_name, ptr);
		
		auto readSize = ptr - start;

		//printf("name: %s\n", m_name);
		//printf("AnimationFile::loadFromMemory: %u\n", readSize);

		return ptr;
	}

	u64 AnimationFile::getCrc() const
	{
		u32 sampleCount = 0;
		for (u32 layerIndex = 0; layerIndex < m_animation.layerCount; ++layerIndex)
		{
			auto &layer = m_animation.layers[layerIndex];

			sampleCount += layer.frameCount;
		}

		auto totalSize = m_animation.layerCount * (sizeof(f32) + sizeof(u32)) + sampleCount * sizeof(AnimationSample) + sizeof(m_name) + sizeof(u32);
		//printf("AnimationFile::crc size: %u\n", totalSize);

		auto memory = std::make_unique<u8[]>(totalSize);
		auto ptr = memory.get();

		ptr = vx::write(ptr, m_animation.layerCount);
		for (u32 layerIndex = 0; layerIndex < m_animation.layerCount; ++layerIndex)
		{
			auto &layer = m_animation.layers[layerIndex];
			ptr = vx::write(ptr, layer.frameCount);
			ptr = vx::write(ptr, layer.frameRate);

			for (u32 i = 0; i < layer.frameCount; ++i)
			{
				ptr = vx::write(ptr, layer.samples[i]);
			}
		}

		ptr = vx::write(ptr, m_name);

		return CityHash64((char*)memory.get(), totalSize);
	}

	u32 AnimationFile::getGlobalVersion()
	{
		return 0;
	}
}