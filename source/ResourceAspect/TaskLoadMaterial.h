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

namespace Graphics
{
	class Texture;
}

namespace vx
{
	class TaskManager;
}

class Material;

template<typename T>
class ResourceManager;

#include "TaskLoadFile.h"
#include <string>
#include <vxLib/StringID.h>
#include <vxEngineLib/managed_ptr.h>

struct TaskLoadMaterialDesc
{
	std::string m_fileNameWithPath;
	shared_ptr<Event> evt;
	ResourceManager<Material>* m_materialManager;
	ResourceManager<Graphics::Texture>* m_textureManager;
	vx::StringID m_sid;
	vx::TaskManager* m_taskManager;
};

class TaskLoadMaterial : public Task
{
	std::string m_fileNameWithPath;
	ResourceManager<Material>* m_materialManager;
	ResourceManager<Graphics::Texture>* m_textureManager;
	vx::StringID m_sid;
	vx::TaskManager* m_taskManager;

	TaskReturnType runImpl() override;

public:
	TaskLoadMaterial(TaskLoadMaterialDesc &&desc);
	~TaskLoadMaterial();

	f32 getTimeMs() const override { return 0.0f; }
};