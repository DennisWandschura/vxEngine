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
	std::string m_filename;
	Event evt;
	vx::StringID m_sid;
	ResourceManager<Graphics::Texture>* m_textureManager;
	bool m_flipImage;
	bool m_srgb;
};

class TaskLoadTexture : public TaskLoadFile
{
	ResourceManager<Graphics::Texture>* m_textureManager;
	std::string m_filename;
	vx::StringID m_sid;
	bool m_flipImage;
	bool m_srgb;

	TaskReturnType runImpl() override;

public:
	TaskLoadTexture(TaskLoadTextureDesc &&desc);
	~TaskLoadTexture();

	f32 getTimeMs() const override;
};