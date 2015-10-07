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
	u8 m_flipImage;
	u8 m_srgb;
};

class TaskLoadTexture : public TaskLoadFile
{
	ResourceManager<Graphics::Texture>* m_textureManager;
	std::string m_filename;
	vx::StringID m_sid;
	u8 m_flipImage;
	u8 m_srgb;

	TaskReturnType runImpl() override;

public:
	TaskLoadTexture(TaskLoadTextureDesc &&desc);
	~TaskLoadTexture();

	f32 getTimeMs() const override;

	const char* getName(u32* size) const override
	{
		*size = 16;
		return "TaskLoadTexture";
	}
};