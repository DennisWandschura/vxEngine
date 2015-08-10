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

class Scene;
class Material;

namespace vx
{
	class MeshFile;
	class TaskManager;
	struct Animation;
	class FileEntry;
}

template<typename T>
class ResourceManager;

#include <vxEngineLib/Task.h>
#include <string>
#include <vxLib/Allocator/StackAllocator.h>

struct TaskLoadSceneDesc
{
	std::string m_filenameWithPath;
	ResourceManager<vx::MeshFile>* m_meshManager;
	ResourceManager<Material>* m_materialManager;
	ResourceManager<vx::Animation>* m_animationManager;
	Scene* m_scene;
	vx::TaskManager* m_taskManager;
	shared_ptr<Event> m_evt;
};

class TaskLoadScene : public Task
{
	std::string m_filenameWithPath;
	ResourceManager<vx::MeshFile>* m_meshManager;
	ResourceManager<Material>* m_materialManager;
	ResourceManager<vx::Animation>* m_animationManager;
	Scene* m_scene;
	vx::StackAllocator m_scratchAllocator;
	vx::TaskManager* m_taskManager;

	bool loadFile(u8** data, u32* fileSize);

	TaskReturnType runImpl() override;

	void createTaskLoadMesh(const vx::FileEntry &it, std::vector<shared_ptr<Event>>* events);
	void createTaskLoadAnimation(const vx::FileEntry &it, std::vector<shared_ptr<Event>>* events);
	void createTaskLoadMaterial(const vx::FileEntry &it, std::vector<shared_ptr<Event>>* events);

public:
	TaskLoadScene(TaskLoadSceneDesc &&rhs);
	~TaskLoadScene();

	f32 getTimeMs() const override { return 0.0f; }
};