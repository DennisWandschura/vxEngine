#pragma once

template<typename T>
class ResourceManager;

namespace Graphics
{
	class Texture;
}

namespace vx
{
	class MessageManager;
}

#include "TaskLoadFile.h"
#include <vxLib/StringID.h>

struct TaskLoadTextureDesc
{
	std::string m_fileNameWithPath;
	Event evt;
	vx::StringID m_sid;
	ResourceManager<Graphics::Texture>* m_textureManager;
};

class TaskLoadTexture : public TaskLoadFile
{
	ResourceManager<Graphics::Texture>* m_textureManager;
	vx::StringID m_sid;

	TaskReturnType runImpl() override;

public:
	TaskLoadTexture(TaskLoadTextureDesc &&desc);
	~TaskLoadTexture();

	f32 getTimeMs() const override { return 0.0f; }
};